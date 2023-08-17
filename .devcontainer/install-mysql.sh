echo "Building mysql connector"

sudo apt-get update
sudo apt-get install libssl-dev libmysqlclient-dev
wget https://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-8.0.26-src.tar.gz
tar -xvf mysql-connector-c++-8.0.26-src.tar.gz
cd mysql-connector-c++-8.0.26-src
cmake .
make
sudo make install
rm ../mysql-connector-c++-8.0.26-src.tar.gz