#!/bin/bash -e


echo "我的名字是 `basename $0` - 我是调用自 $0"
echo "我有 $# 参数xcxx"

count=1
while [ "$#" -ge "1" ];do
    echo "参数序号为 $count 是 $1"
	path=$1
    let count=count+1
    shift
done   


add_tail="c"
echo "path = $path"
 path_bk=$path"d"
 echo "path_bk = $path_bk"

cat $path > $path_bk
rm -f $path
mv $path_bk $path




