BASE_DIR=$(shell realpath ./)
SRC_DIR=$(BASE_DIR)/src
TARGET_DIR=$(BASE_DIR)/target
GEN_DIR=$(TARGET_DIR)/generated
BUILD_DIR=$(TARGET_DIR)/build
DEPS_DIR=$(SRC_DIR)/deps

CMAKE_CMD=cmake
CMAKE_GENERATOR=Unix Makefiles
MAKE_CMD=make
CXX=g++
BUILD_TYPE=Release

ENABLE_FLATBUFFER=1
ENABLE_GTEST=1

PREFIX=/usr/local

ifeq ($(ENABLE_GTEST), 0)

test: build

else

test: build ${BUILD_DIR}/src/test/cpp/wtk-test
	${BUILD_DIR}/src/test/cpp/wtk-test

endif

build: gen_parser | configure $(BUILD_DIR)
	cd $(BUILD_DIR) && $(MAKE_CMD) -j4
	cp $(BUILD_DIR)/src/main/cpp/wtk-* $(TARGET_DIR)/

static-analysis: | configure
	cd $(BUILD_DIR) && $(MAKE_CMD) static-analysis

regression-test: build
	PYTHONPATH=src/main/python python3 src/test/python/regression_tests.py

ifeq ($(ENABLE_FLATBUFFER), 0)
FLATBUFFER_CONFIG=""
else
FLATBUFFER_CONFIG= \
  -DFLATBUFFERS_BUILD_TESTS=OFF \
  -DFLATBUFFERS_BUILD_FLATC=OFF \
  -DFLATBUFFERS_BUILD_FLATHASH=OFF \
  -DFLATBUFFERS_BUILD_SHAREDLIB=OFF
endif

configure: | $(TARGET_DIR)/configure.success

$(TARGET_DIR)/configure.success: | gen_parser $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(CMAKE_CMD) -G "$(CMAKE_GENERATOR)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-DCMAKE_CXX_COMPILER=$(shell which $(CXX)) \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
		-DENABLE_FLATBUFFER=$(ENABLE_FLATBUFFER) \
		-DENABLE_GTEST=$(ENABLE_GTEST) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(FLATBUFFER_CONFIG) \
		$(BASE_DIR) \
	;
	echo 'success' > $(TARGET_DIR)/configure.success

gen_parser: deps $(GEN_DIR)/wtk/irregular/automatas.i.h gen_flatbuffer

$(GEN_DIR)/wtk/irregular/automatas.i.h: src/main/python/automatagen.py \
	src/main/python/dfa.py \
	| $(GEN_DIR)/wtk/irregular
	python3 src/main/python/automatagen.py

ifeq ($(ENABLE_FLATBUFFER), 0)

gen_flatbuffer:
	true

else

gen_flatbuffer: $(GEN_DIR)/wtk/flatbuffer/sieve_ir_generated.h

FLATC_TOOL=$(DEPS_DIR)/flatbuffer/build/flatc --cpp --cpp-std c++11

$(GEN_DIR)/wtk/flatbuffer/sieve_ir_generated.h: \
	$(SRC_DIR)/main/fbs/sieve_ir.fbs \
	| $(GEN_DIR)/wtk/flatbuffer
	cd $(GEN_DIR)/wtk/flatbuffer && \
		$(FLATC_TOOL) $(SRC_DIR)/main/fbs/sieve_ir.fbs

$(GEN_DIR)/wtk/flatbuffer: | $(GEN_DIR)
	mkdir -p $(GEN_DIR)/wtk/flatbuffer/

endif

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

$(BUILD_DIR): | $(TARGET_DIR)
	mkdir -p $(BUILD_DIR)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(GEN_DIR): | $(TARGET_DIR)
	mkdir -p $(GEN_DIR)

$(GEN_DIR)/wtk/irregular: | $(GEN_DIR)
	mkdir -p $(GEN_DIR)/wtk/irregular

install:
	cd $(BUILD_DIR) && $(MAKE_CMD) install

uninstall:
	xargs rm < $(BUILD_DIR)/install_manifest.txt

clean:
	rm -rf $(TARGET_DIR)

deps-clean: clean
	cd $(DEPS_DIR) && \
	rm -rf */ && \
	rm -f deps.success

deps: | $(DEPS_DIR)/deps.success

$(DEPS_DIR)/deps.success: | $(DEPS_DIR)
	cd $(DEPS_DIR) && \
		./logging-install.sh && \
		./gtest-install.sh $(ENABLE_GTEST) && \
		mkdir sst_bignum && \
		cd sst_bignum && \
		../sst_bignum_only.sh && \
	  cd ../ && \
		./flatbuffer-install.sh $(ENABLE_FLATBUFFER) $(CMAKE_CMD) && \
		echo 'success' > deps.success
