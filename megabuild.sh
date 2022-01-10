# first of all setup the path to sources 
build_root_dir=<path_to_sources>
build_kernels=$(ls /lib/modules)  # for a build for nonexisting kernels just use: /usr/src/kernels if the kernels are there

if [ -d $build_root_dir ]
then
	rm -rf $build_root_dir/build_dir
	rm -rf $build_root_dir/dist
	mkdir $build_root_dir/dist
	mkdir $build_root_dir/build_dir

	for dist in $build_kernels
	do

		dist=$(basename $dist)

		cp -r $build_root_dir/src/module/* $build_root_dir/build_dir
		cd $build_root_dir/build_dir
		make KBUILD_DIR=/lib/modules/$dist/build
		compiled=$?
		cd ..
		if [ $compiled -ne 0 ]
		then
			echo "Unable to compile $dist"
		else 
			mkdir $build_root_dir/dist/$dist
			cp $build_root_dir/build_dir/*.ko $build_root_dir/dist/$dist
		fi
	done
else
	echo "build_root_dir not found"
fi
