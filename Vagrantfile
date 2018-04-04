Vagrant.configure("2") do |config|
  config.vm.network "public_network", ip: "x.x.x.x", bridge: "eth0"
  config.vm.box = "ubuntu/xenial64"
  config.vm.provider "virtualbox" do |vb|
     vb.memory = "4096"
     vb.name = "opt_jr"
     vb.customize ["modifyvm", :id, "--ioapic", "on"]
     vb.customize ["modifyvm", :id, "--cpus", "8"]
  end
  config.vm.provision "shell", inline: <<-SHELL
     adduser work --gecos "First Last,RoomNumber,WorkPhone,HomePhone" --disabled-password
     adduser work sudo
     export DEBIAN_FRONTEND=noninteractive
     apt -y update
     apt -y install gcc make openmpi-bin openmpi-doc libopenmpi-dev libmysqlclient-dev
     cd /home/work
     git clone https://github.com/eubr-bigsea/opt_jr
     cd opt_jr
     git checkout wip
     cd Release
     make
     cd /home/work
     chown -R work:work opt_jr
     cp /home/work/opt_jr/wsi_config.xml /home/work/wsi_config.xml
     chown work:work wsi_config.xml
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
     git clone https://github.com/eubr-bigsea/OPT_DATA_CONFIG
     tar zxvf LogP8_1.tar.gz
     tar zxvf LogP8_2.tar.gz
     tar zxvf LogP8_3.tar.gz
     tar zxvf LogP8_4.tar.gz
     cd /home/work
     chown -R work:work OPT_DATA_CONFIG
     git clone https://github.com/eubr-bigsea/dagSim
     cd dagSim
     make
     chmod ugo+x dagsim.sh
     mkdir /home/work/Dagsim
     cp -r ./* /home/work/Dagsim
     cd /home/work
     chown -R work:work dagSim
     chown -R work:work Dagsim
     route add default gw x.x.x.x 
     route del default gw 10.0.2.2
  SHELL
end
