��ý�岿��
1.������ý�������
  ��1������nginx-video-1.6.2.tar.gz����ѹ
  ��2��ִ��./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  ��3����nginx.stream.conf�ļ�������/usr/local/nginx/conf/nginx.confĿ¼��
  ��4������nginx.conf�����ļ��е��ļ��У�����/home/live
2.����stream_server����
    a.����streamserver.stream.tar.gz, ��ѹ
	b. �����������ļ� etc.xml  server.conf  stream_server
	   server.conf   - ��־�����ļ�
	   etc.xml       - ���������ļ�
	   stream_server - ����
	   etc.xml�����ļ�˵����
	   		- <server_ip>27.223.102.92</server_ip>#����˳��������ý�������ip��ַ
			- <server_port>4188</server_port>#�˳���stream_server����Ҫ�Ķ˿� ����server_ip server_portΪһ�ԣ�����������ip�������˿�
			- <stream_server_ip>27.223.92.102</stream_server_ip>#web��������ַ������ip
			- <stream_server_http_port>4180</stream_server_http_port>#web�������˿ڣ������˿�
			- <mysql_ip>27.223.92.102</mysql_ip>#���ݿ��ַ������ip ������ý������������ô��
			- <mysql_port>13306</mysql_port>#���ݿ�˿ڣ������˿ڣ�����ý������������ô��
			- <mysql_user>root</mysql_user>#���ݿ��½�û���������ý������������ô��
			- <mysql_passwd>RbF2017_$</mysql_passwd>#���ݿ��½���루����ý������������ô��
	   
�㲥����������
1.������ý�������
  ��1������nginx-video-1.6.2.tar.gz����ѹ
  ��2��ִ��./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  ��3����nginx.conf�ļ�������/usr/local/nginx/conf/nginx.confĿ¼��
  ��4������nginx.vod.conf�����ļ��е��ļ��У�����/home/vod
2.����stream_server����
    a.����streamserver.vod.tar.gz, ��ѹ
	b. �����������ļ� etc.xml  server.conf  stream_server
	   server.conf   - ��־�����ļ�
	   etc.xml       - ���������ļ�
	   stream_server - ���� 
	   etc.xml�����ļ�˵����	   
	   		- <server_ip>27.223.102.92</server_ip>#����ý�������ip��ַ
			- <server_port>4188</server_port>#����ý���������stream_server�Ķ˿� ����server_ip server_portΪһ�ԣ�����������ip�������˿�
			- <vod_server_ip>27.223.92.102</store_server_ip>#�˷�����Ϊ�ؿ�����������������Ϊ�˻ؿ�������ip��ַ������ip
			- <vod_m3u8_port>4166</store_m3u8_port>#�˷�����Ϊ�ؿ�����������������Ϊ�˻ؿ�������hls���Ŷ˿ڣ������˿�
			- <m3u8_path>/home/vod</m3u8_path>#hls����Ŀ¼
�洢�岿��
1.������ý�������
  ��1������nginx-video-1.6.2.tar.gz����ѹ
  ��2��ִ��./configure --prefix=/home/tmp/ --with-http_ssl_module --user=nobody --group=nobody --with-http_stub_status_module --with-http_ssl_module --with-pcre=./pcre-8.36 --with-zlib=./zlib-1.2.8 --with-http_stub_status_module --add-module=./nginx_ajp_module-master --add-module=./nginx-upload-module-2.2 --add-module=./nginx-rtmp-module-1.2.1
  ��3����nginx.store.conf�ļ�������/usr/local/nginx/conf/nginx.confĿ¼��
  ��4������nginx.conf�����ļ��е��ļ��У�����/home/record
2.����stream_server����
    a.����streamserver.store.tar.gz, ��ѹ
	b. �����������ļ� etc.xml  server.conf  stream_server
	   server.conf   - ��־�����ļ�
	   etc.xml       - ���������ļ�
	   stream_server - ����	  
	   etc.xml�����ļ�˵����
	   		- <server_ip>27.223.102.92</server_ip>#����ý�������ip��ַ
			- <server_port>4188</server_port>#����ý���������stream_server�Ķ˿� ����server_ip server_portΪһ�ԣ�����������ip�������˿�
			- <store_server_ip>27.223.92.102</store_server_ip>#�˷�����Ϊ�洢����������������Ϊ�˴��������ip��ַ������ip
			- <store_download_port>4167</store_download_port>#�˷�����Ϊ�洢����������������Ϊ�˴洢�������ļ����ض˿ڣ������˿�
			- <record_path>/home/record</record_path>#hls����Ŀ¼	   
  
