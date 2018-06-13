clear
file="/home/dinar/.config/unity3d/Udacity/self_driving_car_nanodegree_program/Player.log"

if [ -f $file ] ; then
    rm -r $file
fi

cmake ..
make -j `nproc` $*
./pid


