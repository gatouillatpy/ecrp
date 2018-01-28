
#include <gcrypt.h>

#include "Crypto.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		static void	show_sexp(const char* prefix, gcry_sexp_t a) {
			fputs(prefix, stderr);
			size_t size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
			char* buf = (char*)malloc(size);
			gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
			printf("%.*s", (int)size, buf);
		}

		static void hash(int algo, void* inputData, size_t inputSize, void* outputData, size_t outputSize) {
			gcry_md_hd_t hd;
			gcry_md_open(&hd, algo, 0);
			gcry_md_write(hd, inputData, inputSize);
			byte* hash = gcry_md_read(hd, algo);
			memcpy(outputData, hash, outputSize);
			gcry_md_close(hd);
		}

		b256 sha256(void* inputData, size_t inputSize) {
			b256 outputData;
			hash(GCRY_MD_SHA256, inputData, inputSize, &outputData, sizeof(outputData));
			return outputData;
		}

		PrivateKey generatePrivateKey() {
			PrivateKey output;
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			gcry_sexp_t key_spec;
			err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)))");
			if (err) {
				printf("Creating S-expression failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t key_pair;
			err = gcry_pk_genkey(&key_pair, key_spec);
			if (err) {
				printf("Creating ECC key failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t private_key;
			private_key = gcry_sexp_find_token(key_pair, "private-key", 0);
			if (!private_key) {
				printf("Private part missing in key.\n");
				return output;
			}
			show_sexp("private_key:\n", private_key);

			gcry_sexp_t q_component;
			q_component = gcry_sexp_find_token(private_key, "q", 0);
			if (!q_component) {
				printf("Q component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(q_component, 1, &bufferSize);
			memcpy(&output.q, buffer, sizeof(output.q));

			gcry_sexp_t d_component;
			d_component = gcry_sexp_find_token(private_key, "d", 0);
			if (!d_component) {
				printf("D component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(d_component, 1, &bufferSize);
			memcpy(&output.d, buffer, sizeof(output.d));

			gcry_sexp_release(d_component);
			gcry_sexp_release(q_component);
			gcry_sexp_release(private_key);
			gcry_sexp_release(key_pair);
			gcry_sexp_release(key_spec);

			return output;
		}

		Signature signData(void* inputData, size_t inputSize, const PrivateKey& privateKey) {
			Signature output;
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			static const char private_key_format[] =
				"(private-key\n"
				"  (ecc\n"
				"    (curve Ed448)\n"
				"      (flags eddsa)\n"
				"      (q %b)\n"
				"      (d %b)\n"
				"  )\n"
				")\n";

			gcry_sexp_t private_key;
			err = gcry_sexp_build(&private_key, NULL, private_key_format, sizeof(privateKey.q), &privateKey.q, sizeof(privateKey.d), &privateKey.d);
			if (!private_key) {
				printf("Loading private key failed: %s\n", gcry_strerror(err));
				return output;
			}
			show_sexp("private_key:\n", private_key);

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, inputData);
			if (err) {
				printf("Loading data failed: %s\n", gcry_strerror(err));
				return output;
			}
			show_sexp("data:\n", data);

			gcry_sexp_t signature;
			err = gcry_pk_sign(&signature, data, private_key);
			if (err) {
				printf("Signing data failed: %s\n", gcry_strerror(err));
				return output;
			}
			show_sexp("signature:\n", signature);

			gcry_sexp_t r_component;
			r_component = gcry_sexp_find_token(signature, "r", 0);
			if (!r_component) {
				printf("R component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(r_component, 1, &bufferSize);
			memcpy(&output.r, buffer, sizeof(output.r));

			gcry_sexp_t s_component;
			s_component = gcry_sexp_find_token(signature, "s", 0);
			if (!s_component) {
				printf("S component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(s_component, 1, &bufferSize);
			memcpy(&output.s, buffer, sizeof(output.s));

			gcry_sexp_release(s_component);
			gcry_sexp_release(r_component);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);
			gcry_sexp_release(private_key);

			return output;
		}

		bool verifyData(void* inputData, size_t inputSize, const Signature& inputSignature, const PublicKey& publicKey) {
			gpg_error_t err;

			static const char public_key_format[] =
				"(public-key\n"
				"  (ecc\n"
				"    (curve Ed448)\n"
				"      (flags eddsa)\n"
				"      (q %b)\n"
				"  )\n"
				")\n";

			gcry_sexp_t public_key;
			err = gcry_sexp_build(&public_key, NULL, public_key_format, sizeof(publicKey.q), &publicKey.q);
			if (!public_key) {
				printf("Loading public key failed: %s\n", gcry_strerror(err));
				return false;
			}
			show_sexp("public_key:\n", public_key);

			static const char signature_format[] =
				"(signature\n"
				"  (sig-val\n"
				"    (eddsa\n"
				"      (r %b)\n"
				"      (s %b)\n"
				"    )\n"
				"  )\n"
				")\n";

			gcry_sexp_t signature;
			err = gcry_sexp_build(&signature, NULL, signature_format, sizeof(inputSignature.r), &inputSignature.r, sizeof(inputSignature.s), &inputSignature.s);
			if (!signature) {
				printf("Loading signature failed: %s\n", gcry_strerror(err));
				return false;
			}
			show_sexp("signature:\n", signature);

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, inputData);
			if (err) {
				printf("Loading data failed: %s\n", gcry_strerror(err));
				return false;
			}
			show_sexp("data:\n", data);

			err = gcry_pk_verify(signature, data, public_key);
			if (err) {
				printf("Verifying data failed: %s\n", gcry_strerror(err));
				return false;
			}

			gcry_sexp_release(public_key);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);

			return true;
		}
	}
}