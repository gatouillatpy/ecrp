ZLIB        = zlib-1.2.8
BOOST       = /cygdrive/c/boost_1_66_0
GCRYPT      = /cygdrive/c/libgcrypt-1.8.2/dist/lib/cygwin_x86
GPG_ERROR   = /cygdrive/c/libgpg-error-1.27/dist/lib/cygwin_x86
INTL        = /cygdrive/d/cygwin64/usr/i686-pc-cygwin/sys-root/usr/lib

INCLUDES    = -I$(ZLIB) -I$(BOOST) -Isrc
TARGET      = ecrp

CPPFLAGS    = -O3 $(INCLUDES) -pedantic -Wall -std=c++0x -D__STDC_LIMIT_MACROS -DBOOST_SYSTEM_NO_DEPRECATED
LDFLAGS     = -L$(ZLIB) -L$(GCRYPT) -L$(GPG_ERROR) -L$(INTL) -lstdc++ -lz -lboost_system -lboost_filesystem -lboost_thread -l:libintl2.a -l:libgcrypt.a -l:libgpg-error.a

.SUFFIXES: .cpp .o

SOURCES = \
src/bank/Bank.cpp \
src/bank/Wallet.cpp \
src/blockchain/transactions/BasicTransaction.cpp \
src/blockchain/transactions/TransactionInput.cpp \
src/blockchain/transactions/TransactionOutput.cpp \
src/blockchain/Block.cpp \
src/blockchain/Blockchain.cpp \
src/blockchain/MasterBlock.cpp \
src/blockchain/Transaction.cpp \
src/crypto/Crypto.cpp \
src/errors/Error.cpp \
src/utils/compression.cpp \
src/utils/utils.cpp \
src/utils/varints.cpp \
src/ECRP_Test.cpp \

OBJECTS = ${SOURCES:.cpp=.o}

all: $(OBJECTS)
	g++ $(OBJECTS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp 
	g++ $(CPPFLAGS) -o $@ -c $<

clean:
	/bin/rm -rf $(TARGETS) $(OBJECTS) $(TARGET)
