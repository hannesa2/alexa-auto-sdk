# iterate all project to upload, indicated by "uploadArchives" task

grep uploadArchives ./platforms -lR | xargs -n1 dirname | while read -r module ; do
   pushd $module
   gradle uploadArchives
   popd 1>/dev/null
done