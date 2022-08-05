BASE_DIR=$(shell realpath ./)
SRC_DIR=$(BASE_DIR)/src
TARGET_DIR=$(BASE_DIR)/target
GEN_DIR=$(TARGET_DIR)/generated
BUILD_DIR=$(TARGET_DIR)/build
DEPS_DIR=$(SRC_DIR)/deps

ANTLR_TOOL=java -jar $(DEPS_DIR)/antlr4/antlr-tool.jar -Dlanguage=Cpp -no-visitor -no-listener
FLATC_TOOL=$(DEPS_DIR)/flatbuffer/build/flatc --cpp --cpp-std c++11
CMAKE_CMD=cmake
CMAKE_GENERATOR=Unix Makefiles
MAKE_CMD=make
CXX=g++
BUILD_TYPE=Release

PREFIX=/usr/local

ENABLE_ANTLR=1
ENABLE_FLATBUFFER=1

test: build
	cd target/build/src/test/cpp/ && ./wtk-test
	python3 -u src/main/python/test_suite.py

static-analysis: | configure
	cd $(BUILD_DIR) && $(MAKE_CMD) static-analysis

build: gen_parser | configure $(BUILD_DIR)
	cd $(BUILD_DIR) && $(MAKE_CMD) -j4
	cp $(BUILD_DIR)/src/main/cpp/wtk-* $(TARGET_DIR)/
	echo 'success' > $(BUILD_DIR)/build.success

configure: | $(TARGET_DIR)/configure.success

ifeq ($(ENABLE_FLATBUFFER), 0)
FLATBUFFER_CONFIG=""
else
FLATBUFFER_CONFIG= \
  -DFLATBUFFERS_BUILD_TESTS=OFF \
  -DFLATBUFFERS_BUILD_FLATC=OFF \
  -DFLATBUFFERS_BUILD_FLATHASH=OFF \
  -DFLATBUFFERS_BUILD_SHAREDLIB=OFF
endif

$(TARGET_DIR)/configure.success: | gen_parser $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(CMAKE_CMD) -G "$(CMAKE_GENERATOR)" \
			-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
			-DCMAKE_CXX_COMPILER=$(shell which $(CXX)) \
			-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
			-DENABLE_ANTLR=$(ENABLE_ANTLR) \
			-DENABLE_FLATBUFFER=$(ENABLE_FLATBUFFER) \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
			$(FLATBUFFER_CONFIG) \
			$(BASE_DIR) \
	;
	echo 'success' > $(TARGET_DIR)/configure.success

install:
	cd $(BUILD_DIR) && $(MAKE_CMD) install

clean:
	rm -rf $(TARGET_DIR)

deps-clean: clean
	cd $(DEPS_DIR) && \
	rm -rf */ && \
	rm -f deps.success

deps: | $(DEPS_DIR)/deps.success

$(DEPS_DIR)/deps.success: | $(DEPS_DIR)
	cd $(DEPS_DIR) && \
		mkdir -p sst_bignum && \
		cd sst_bignum && \
		../sst_bignum_only.sh
	cd $(DEPS_DIR) && \
	./antlr4-install.sh $(ENABLE_ANTLR) && \
	./gtest-install.sh && \
	./logging-install.sh && \
	./flatbuffer-install.sh $(ENABLE_FLATBUFFER) $(CMAKE_CMD) && \
	echo 'success' > deps.success

gen_parser: deps \
	$(GEN_DIR)/wtk/irregular/Automatas.h \
	gen_antlr \
	gen_flatbuffer

ifeq ($(ENABLE_ANTLR), 0)

gen_antlr:
	true

else

gen_antlr: $(GEN_DIR)/wtk/antlr/SIEVEIRParser.h

$(GEN_DIR)/wtk/antlr/SIEVEIRParser.h: \
	src/main/g4/SIEVEIR.g4 \
	| $(GEN_DIR)/wtk/antlr
	cd src/main/g4 && \
		$(ANTLR_TOOL) -package wtk_gen_antlr -o $(GEN_DIR)/wtk/antlr SIEVEIR.g4

endif

ifeq ($(ENABLE_FLATBUFFER), 0)

gen_flatbuffer:
	true

else

gen_flatbuffer: $(GEN_DIR)/wtk/flatbuffer/sieve_ir_generated.h

$(GEN_DIR)/wtk/flatbuffer/sieve_ir_generated.h: \
	$(SRC_DIR)/main/fbs/sieve_ir.fbs \
	| $(GEN_DIR)/wtk/flatbuffer
	cd $(GEN_DIR)/wtk/flatbuffer/ && \
		$(FLATC_TOOL) $(SRC_DIR)/main/fbs/sieve_ir.fbs

endif

$(GEN_DIR)/wtk/irregular/Automatas.h: \
	src/main/python/automatagen.py \
	| $(GEN_DIR)/wtk/irregular
	python3 src/main/python/automatagen.py

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(GEN_DIR): | $(TARGET_DIR)
	mkdir -p $(GEN_DIR)

$(GEN_DIR)/wtk/antlr: | $(GEN_DIR)
	mkdir -p $(GEN_DIR)/wtk/antlr

$(GEN_DIR)/wtk/irregular: | $(GEN_DIR)
	mkdir -p $(GEN_DIR)/wtk/irregular

$(GEN_DIR)/wtk/flatbuffer: | $(GEN_DIR)
	mkdir -p $(GEN_DIR)/wtk/flatbuffer

$(BUILD_DIR): | $(TARGET_DIR)
	mkdir -p $(BUILD_DIR)

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)
