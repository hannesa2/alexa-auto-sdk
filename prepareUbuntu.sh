git clone git://git.openembedded.org/openembedded-core oe-core -b rocko
cd oe-core || exit
git clone git://git.openembedded.org/bitbake -b 1.36
export OE_CORE_PATH=$(pwd)
echo OE_CORE_PATH=$OE_CORE_PATH
sudo apt-get install chrpath diffstat gawk texinfo python python3 wget unzip build-essential cpio git-core libssl-dev quilt cmake libsqlite3-dev libarchive-dev python3-dev libdb-dev libpopt-dev zlib1g-dev
sudo apt-get install libssl-dev
sudo apt-get remove docker docker-engine docker.io containerd runc
sudo apt-get update
sudo apt-get install apt-transport-https ca-certificates curl gnupg lsb-release
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io