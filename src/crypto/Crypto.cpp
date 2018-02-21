
#include "Crypto.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		const char format_E168_generateKey[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve E168)\n"
			"    (flags eddsa)\n"
		    "  )\n"
			")\n";
		const char format_E168_generateKey_withSecret[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve E168)\n"
			"    (flags eddsa)\n"
			"    (secret %b)\n"
			"  )\n"
			")\n";
		const char format_E168_signData_privateKey[] =
			"(private-key\n"
			"  (ecc\n"
			"    (curve E168)\n"
			"    (flags eddsa)\n"
			"    (q %b)\n"
			"    (d %b)\n"
			"  )\n"
			")\n";
		const char format_E168_verifyData_publicKey[] =
			"(public-key\n"
			"  (ecc\n"
			"    (curve E168)\n"
			"      (flags eddsa)\n"
			"      (q %b)\n"
			"  )\n"
			")\n";
		const char format_E168_verifyData_signature[] =
			"(signature\n"
			"  (sig-val\n"
			"    (eddsa\n"
			"      (r %b)\n"
			"      (s %b)\n"
			"    )\n"
			"  )\n"
			")\n";
		const char format_E168_data[] =
			"(data\n"
			"  (flags eddsa)\n"
			"  (hash-algo shake256)\n"
			"  (value %b)\n"
			")\n";

		const char format_Ed25519_generateKey[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve Ed25519)\n"
			"    (flags eddsa)\n"
			"  )\n"
			")\n";
		const char format_Ed25519_generateKey_withSecret[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve Ed25519)\n"
			"    (flags eddsa)\n"
			"    (secret %b)\n"
			"  )\n"
			")\n";
		const char format_Ed25519_signData_privateKey[] =
			"(private-key\n"
			"  (ecc\n"
			"    (curve Ed25519)\n"
			"    (flags eddsa)\n"
			"    (q %b)\n"
			"    (d %b)\n"
			"  )\n"
			")\n";
		const char format_Ed25519_verifyData_publicKey[] =
			"(public-key\n"
			"  (ecc\n"
			"    (curve Ed25519)\n"
			"      (flags eddsa)\n"
			"      (q %b)\n"
			"  )\n"
			")\n";
		const char format_Ed25519_verifyData_signature[] =
			"(signature\n"
			"  (sig-val\n"
			"    (eddsa\n"
			"      (r %b)\n"
			"      (s %b)\n"
			"    )\n"
			"  )\n"
			")\n";
		const char format_Ed25519_data[] =
			"(data\n"
			"  (flags eddsa)\n"
			"  (hash-algo sha512)\n"
			"  (value %b)\n"
			")\n";

		const char format_Ed448_generateKey[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve Ed448)\n"
			"    (flags eddsa)\n"
			"  )\n"
			")\n";
		const char format_Ed448_generateKey_withSecret[] =
			"(genkey\n"
			"  (ecdsa\n"
			"    (curve Ed448)\n"
			"    (flags eddsa)\n"
			"    (secret %b)\n"
			"  )\n"
			")\n";
		const char format_Ed448_signData_privateKey[] =
			"(private-key\n"
			"  (ecc\n"
			"    (curve Ed448)\n"
			"    (flags eddsa)\n"
			"    (q %b)\n"
			"    (d %b)\n"
			"  )\n"
			")\n";
		const char format_Ed448_verifyData_publicKey[] =
			"(public-key\n"
			"  (ecc\n"
			"    (curve Ed448)\n"
			"      (flags eddsa)\n"
			"      (q %b)\n"
			"  )\n"
			")\n";
		const char format_Ed448_verifyData_signature[] =
			"(signature\n"
			"  (sig-val\n"
			"    (eddsa\n"
			"      (r %b)\n"
			"      (s %b)\n"
			"    )\n"
			"  )\n"
			")\n";
		const char format_Ed448_data[] =
			"(data\n"
			"  (flags eddsa)\n"
			"  (hash-algo shake256)\n"
			"  (value %b)\n"
			")\n";

		static void show_sexp(const char* prefix, gcry_sexp_t a) {
			fputs(prefix, stderr);
			size_t size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
			char* buf = (char*)malloc(size);
			gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
			printf("%.*s", (int)size, buf);
		}

		static void hash(int algo, const void* pInputData, size_t inputSize, void* pOutputData, size_t outputSize) {
			gcry_md_hd_t hd;
			gpg_error_t err;
			err = gcry_md_open(&hd, algo, 0);
			if (err) {
				throw Error("Initializing hash algorithm failed: %d", err);
			}
			gcry_md_write(hd, pInputData, inputSize);
			byte* hash = gcry_md_read(hd, algo);
			if (hash == NULL) {
				gcry_md_close(hd);
				throw Error("Reading hash failed.");
			}
			memcpy(pOutputData, hash, outputSize);
			gcry_md_close(hd);
		}

		b256 sha256(const void* pInputData, size_t inputSize) {
			b256 outputData;
			hash(GCRY_MD_SHA256, pInputData, inputSize, &outputData, sizeof(outputData));
			return outputData;
		}
	}
}