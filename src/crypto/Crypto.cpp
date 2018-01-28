
#include <gcrypt.h>

#include "Crypto.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		static void hash(int algo, void* inputData, size_t inputSize, void* outputData, size_t outputSize) {
			gcry_md_hd_t hd;
			gcry_md_open(&hd, algo, 0);
			gcry_md_write(hd, inputData, inputSize);
			byte* hash = gcry_md_read(hd, algo);
			memcpy(outputData, hash, outputSize);
			gcry_md_close(hd);
		}

		hash256& sha256(void* inputData, size_t inputSize) {
			hash256 outputData;
			hash(GCRY_MD_SHA256, inputData, inputSize, &outputData, sizeof(outputData));
			return outputData;
		}

		static void	show_sexp(const char* prefix, gcry_sexp_t a) {
			fputs(prefix, stderr);
			size_t size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
			char* buf = (char*)malloc(size);
			gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
			printf("%.*s", (int)size, buf);
		}

		static const char sample_eddsa_key_Ed25519[] =
			"(key-data\n"
			"  (public-key\n"
			"    (ecc\n"
			"      (curve Ed25519)\n"
			"	   (flags eddsa)\n"
			"	   (q #EC172B93AD5E563BF4932C70E1245034C35467EF2EFD4D64EBF819683467E2BF#)\n"
			"    )\n"
			"  )\n"
			"  (private-key\n"
			"    (ecc\n"
			"      (curve Ed25519)\n"
			"        (flags eddsa)\n"
			"        (q #EC172B93AD5E563BF4932C70E1245034C35467EF2EFD4D64EBF819683467E2BF#)\n"
			"        (d #833FE62409237B9D62EC77587520911E9A759CEC1D19755B7DA901B96DCA3D42#)\n"
			"    )\n"
			"  )\n"
			")\n";

		static const char sample_eddsa_data_Ed25519[] =
			"(data\n"
			"  (flags eddsa)\n"
			"  (hash-algo sha512)\n"
			"  (value #DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F#)\n"
			")\n";

		static const char sample_eddsa_key_Ed448[] =
			"(key-data\n"
			"  (public-key\n"
			"    (ecc\n"
			"      (curve Ed448)\n"
			"	   (flags eddsa)\n"
			"	   (q #DF9705F58EDBAB802C7F8363CFE5560AB1C6132C20A9F1DD163483A26F8AC53A39D6808BF4A1DFBD261B099BB03B3FB50906CB28BD8A081F00#)\n"
			"    )\n"
			"  )\n"
			"  (private-key\n"
			"    (ecc\n"
			"      (curve Ed448)\n"
			"        (flags eddsa)\n"
			"        (q #DF9705F58EDBAB802C7F8363CFE5560AB1C6132C20A9F1DD163483A26F8AC53A39D6808BF4A1DFBD261B099BB03B3FB50906CB28BD8A081F00#)\n"
			"        (d #D65DF341AD13E008567688BAEDDA8E9DCDC17DC024974EA5B4227B6530E339BFF21F99E68CA6968F3CCA6DFE0FB9F4FAB4FA135D5542EA3F01#)\n"
			"    )\n"
			"  )\n"
			")\n";

		static const char sample_eddsa_data_Ed448[] =
			"(data\n"
			"  (flags eddsa)\n"
			"  (hash-algo shake256)\n"
			"  (value #BD0F6A3747CD561BDDDF4640A332461A4A30A12A434CD0BF40D766D9C6D458E5512204A30C17D1F50B5079631F64EB3112182DA3005835461113718D1A5EF944#)\n"
			")\n";

		void test(void* inputData, size_t inputSize) {
			const int key_size = 256;
			gpg_error_t err;

			gcry_sexp_t key_spec;
			err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)))");
			//err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed25519\")(flags eddsa)))");
			//err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve %s)))", "secp256k1");
			//err = gcry_sexp_build(&key_spec, NULL, "(genkey (ECDSA (nbits %d)))", 256);
			if (err) {
				printf("Creating S-expression failed: %s\n", gcry_strerror(err));
				return;
			}

			gcry_sexp_t key_pair;
			err = gcry_pk_genkey(&key_pair, key_spec);
			if (err) {
				printf("Creating ECC key failed: %s\n", gcry_strerror(err));
				return;
			}
			show_sexp("ECC key:\n", key_pair);

			//err = gcry_sexp_sscan(&key_pair, NULL, sample_eddsa_key_Ed448, strlen(sample_eddsa_key_Ed448));
			//err = gcry_sexp_sscan(&key_pair, NULL, sample_eddsa_key_Ed25519, strlen(sample_eddsa_key_Ed25519));
			if (err) {
				printf("Loading S-expression failed: %s\n", gcry_strerror(err));
				return;
			}

			gcry_sexp_t pub_key;
			pub_key = gcry_sexp_find_token(key_pair, "public-key", 0);
			if (!pub_key) {
				printf("Public part missing in key.\n");
				return;
			}
			show_sexp("Public key:\n", pub_key);

			gcry_sexp_t sec_key;
			sec_key = gcry_sexp_find_token(key_pair, "private-key", 0);
			if (!sec_key) {
				printf("Private part missing in key.\n");
				return;
			}
			show_sexp("Private key:\n", sec_key);

			gcry_sexp_release(key_pair);
			gcry_sexp_release(key_spec);

			gcry_mpi_t x;
			x = gcry_mpi_new(8 * inputSize);
			err = gcry_mpi_scan(&x, GCRYMPI_FMT_HEX, inputData, 0, 0);
			if (err) {
				printf("Reading data failed: %s\n", gcry_strerror(err));
				return;
			}

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, "(data (flags eddsa)(hash-algo shake256)(value %m))", x);
			//err = gcry_sexp_build(&data, NULL, "(data (flags eddsa)(hash-algo sha512)(value %m))", x);
			//err = gcry_sexp_build(&data, NULL, "(data (flags raw)(value %m))", x);
			//err = gcry_sexp_build(&data, NULL, "(data (flags raw)(value %m))", x);
			gcry_mpi_release(x);
			if (err) {
				printf("Converting data failed: %s\n", gcry_strerror(err));
				return;
			}
			show_sexp("Data to sign:\n", data);

			//err = gcry_sexp_sscan(&data, NULL, sample_eddsa_data_Ed448, strlen(sample_eddsa_data_Ed448));
			//err = gcry_sexp_sscan(&data, NULL, sample_eddsa_data_Ed25519, strlen(sample_eddsa_data_Ed25519));
			if (err) {
				printf("Loading S-expression failed: %s\n", gcry_strerror(err));
				return;
			}
			show_sexp("Data to sign:\n", data);

			gcry_sexp_t sig;
			err = gcry_pk_sign(&sig, data, sec_key);
			if (err) {
				printf("Signing data failed: %s\n", gcry_strerror(err));
				return;
			}
			show_sexp("Signature:\n", sig);

			err = gcry_pk_verify(sig, data, pub_key);
			if (err) {
				printf("Verifying data failed: %s\n", gcry_strerror(err));
				return;
			}

			gcry_sexp_release(sig);
			gcry_sexp_release(data);
			gcry_sexp_release(sec_key);
			gcry_sexp_release(pub_key);
		}

	}
}