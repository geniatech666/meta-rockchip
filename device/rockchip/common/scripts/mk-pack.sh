#!/bin/bash -e

RK_PACK_LOONG_DIR="$RK_SDK_DIR/loong"

build_pack_image()
{
	cd $RK_PACK_LOONG_DIR
	./release_all.sh
	cd -
}

# Hooks

usage_hook()
{
	echo -e "pack                          \tpack release"
}

clean_hook()
{
	rm -rf "$RK_PACK_LOONG_DIR/out" 
}

POST_BUILD_CMDS="pack"
post_build_hook()
{
	echo "=========================================="
	echo "          Start packing"
	echo "=========================================="

	build_pack_image
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

post_build_hook $@
