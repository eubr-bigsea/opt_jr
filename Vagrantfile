Vagrant.configure("2") do |config|
#  config.vm.network "public_network", ip: "192.168.0.17"
  config.vm.box = "ubuntu/xenial64"
  config.vm.provider "virtualbox" do |vb|
     vb.memory = "4096"
     vb.name = "opt_jr"
     vb.customize ["modifyvm", :id, "--ioapic", "on"]
     vb.customize ["modifyvm", :id, "--cpus", "8"]
  end
  config.vm.provision "shell", inline: <<-SHELL
     adduser work --gecos "First Last,RoomNumber,WorkPhone,HomePhone" --disabled-password
     export DEBIAN_FRONTEND=noninteractive
     apt -y update
     apt -y install gcc make openmpi-bin openmpi-doc libopenmpi-dev libmysqlclient-dev
     cd /home/work
     git clone https://github.com/eubr-bigsea/opt_jr
     cd opt_jr/Release
     make
     cd /home/work
     chown -R work:work opt_jr
     export temp=$(cat /home/work/opt_jr/wsi_config.xml|grep DB_pass)
     export MYSQL_ROOT_PASSWORD=$(echo $temp| awk -v FS="(>|<)" '{print $3}')
     echo "MYSQL root password is $MYSQL_ROOT_PASSWORD"
     echo "mysql-server mysql-server/root_password password $MYSQL_ROOT_PASSWORD" | debconf-set-selections
     echo "mysql-server mysql-server/root_password_again password $MYSQL_ROOT_PASSWORD" | debconf-set-selections
     apt -y install mysql-server
     git clone https://github.com/eubr-bigsea/wsi.git
     chown -R work:work wsi
     echo "CREATE DATABASE bigsea;" | mysql -uroot -p$MYSQL_ROOT_PASSWORD
     mysql -uroot -p$MYSQL_ROOT_PASSWORD bigsea < /home/work/wsi/Database/creationDB.sql
     mysql -uroot -p$MYSQL_ROOT_PASSWORD bigsea < /home/work/wsi/Database/insertFakeProfile.sql
     mysql -uroot -p$MYSQL_ROOT_PASSWORD bigsea < /home/work/opt_jr/OPTIMIZER_CONFIGURATION_TABLE.sql
     mysql -uroot -p$MYSQL_ROOT_PASSWORD bigsea < /home/work/opt_jr/PREDICTOR_CACHE_TABLE.sql
  SHELL
end
