root_dir=/root/compile/armadillo
rm -rf $root_dir/build_dir
rm -rf $root_dir/dist
mkdir $root_dir/dist
mkdir $root_dir/build_dir

for dist in /usr/src/kernels/*
do
	
	dist=$(basename $dist)

	cp -r $root_dir/module/* $root_dir/build_dir
	cd $root_dir/build_dir
	make KBUILD_DIR=/usr/src/kernels/$dist
	compiled=$?
	cd ..
	if [ $compiled -ne 0 ]
	then
		echo "Unable to compile $dist"
	else 
		mkdir $root_dir/dist/$dist
		cp $root_dir/build_dir/*.ko $root_dir/dist/$dist
	fi
done
