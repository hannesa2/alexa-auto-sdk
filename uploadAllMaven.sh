#!/bin/zsh

find . -name "*.aar"

# iterate all project to upload, indicated by "uploadArchives" task
grep uploadArchives ./platforms -lR | xargs -n1 dirname | while read -r module ; do
   echo "cd $module && gradle uploadArchives && cd ../../../.."
done

grep uploadArchives ./platforms -lR | xargs -n1 dirname > temp.list

cat temp.list | while read -r module ; do
   cd $module
   echo "I'm in $(pwd)"
   gradle uploadArchives | echo "all done"
   set -x
   cd ../../../..
done
