FROM db2client:0.1

# copy bind files
COPY bind /opt/demo/bind
COPY bindsql.sh /opt/demo/bindsql.sh
COPY startbind.sh /opt/demo/startbind.sh
COPY drop_packages.sh /opt/demo/drop_packages.sh
COPY db2catalog.sh /opt/demo/db2catalog.sh


CMD ["/opt/demo/startbind.sh"]
RUN chmod -R 777 /opt/demo

