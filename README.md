流媒体部署
1.部署流媒体服务器
  （1）下载nginx-video-1.6.2.tar.gz，解压
  （2）执行./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  （3）将nginx.live.conf文件拷贝到/usr/local/nginx/conf/nginx.conf目录下
  （4）创建nginx.conf配置文件中的文件夹，例如/home/live
2.部署stream_server服务
    a.下载streamserver.stream.tar.gz, 解压
	b. 其中有三个文件 etc.xml  server.conf  stream_server
	   server.conf   - 日志配置文件
	   etc.xml       - 程序配置文件
	   stream_server - 程序
	   etc.xml配置文件说明：
	   		- <server_ip>27.223.102.92</server_ip>#部署此程序的主流媒体服务器ip地址
			- <server_port>4188</server_port>#此程序stream_server所需要的端口 ！！server_ip server_port为一对，需配置外网ip与外网端口
			- <stream_server_ip>27.223.92.102</stream_server_ip>#web服务器地址，外网ip
			- <stream_server_http_port>4180</stream_server_http_port>#web服务器端口，外网端口
			- <mysql_ip>27.223.92.102</mysql_ip>#数据库地址，外网ip （主流媒体服务器需配置此项）
			- <mysql_port>13306</mysql_port>#数据库端口，外网端口（主流媒体服务器需配置此项）
			- <mysql_user>root</mysql_user>#数据库登陆用户名（主流媒体服务器需配置此项）
			- <mysql_passwd>RbF2017_$</mysql_passwd>#数据库登陆密码（主流媒体服务器需配置此项）
	   
点播服务器部署
1.部署流媒体服务器
  （1）下载nginx-video-1.6.2.tar.gz，解压
  （2）执行./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  （3）将nginx.conf文件拷贝到/usr/local/nginx/conf/nginx.conf目录下
  （4）创建nginx.vod.conf配置文件中的文件夹，例如/home/vod
2.部署stream_server服务
    a.下载streamserver.vod.tar.gz, 解压
	b. 其中有三个文件 etc.xml  server.conf  stream_server
	   server.conf   - 日志配置文件
	   etc.xml       - 程序配置文件
	   stream_server - 程序 
	   etc.xml配置文件说明：	   
	   		- <server_ip>27.223.102.92</server_ip>#主流媒体服务器ip地址
			- <server_port>4188</server_port>#主流媒体服务器的stream_server的端口 ！！server_ip server_port为一对，需配置外网ip与外网端口
			- <vod_server_ip>27.223.92.102</store_server_ip>#此服务器为回看服务器，此配置项为此回看服务器ip地址，外网ip
			- <vod_m3u8_port>4166</store_m3u8_port>#此服务器为回看服务器，此配置项为此回看服务器hls播放端口，外网端口
			- <m3u8_path>/home/vod</m3u8_path>#hls本地目录
存储体部署
1.部署流媒体服务器
  （1）下载nginx-video-1.6.2.tar.gz，解压
  （2）执行./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  （3）将nginx.record.conf文件拷贝到/usr/local/nginx/conf/nginx.conf目录下
  （4）创建nginx.conf配置文件中的文件夹，例如/home/record
2.部署stream_server服务
    a.下载streamserver.store.tar.gz, 解压
	b. 其中有三个文件 etc.xml  server.conf  stream_server
	   server.conf   - 日志配置文件
	   etc.xml       - 程序配置文件
	   stream_server - 程序	  
	   etc.xml配置文件说明：
	   		- <server_ip>27.223.102.92</server_ip>#主流媒体服务器ip地址
			- <server_port>4188</server_port>#主流媒体服务器的stream_server的端口 ！！server_ip server_port为一对，需配置外网ip与外网端口
			- <store_server_ip>27.223.92.102</store_server_ip>#此服务器为存储服务器，此配置项为此储存服务器ip地址，外网ip
			- <store_download_port>4167</store_download_port>#此服务器为存储服务器，此配置项为此存储服务器文件下载端口，外网端口
			- <record_path>/home/record</record_path>#hls本地目录	   
  
stream_server程序部署：
a.将lib.tar.gz此文件解压，把里面的文件拷贝到/usr/local/lib/目录下，命令：cp -d lib/* /usr/local/lib/
b.将lib.conf拷贝到/etc/ld.so.conf.d/，然后执行命令ldconfig
c.修改配置文件
d.启动程序
