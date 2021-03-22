FROM db2client:0.1

# copy server
COPY server /opt/demo/server
COPY startserver.sh /opt/demo/startserver.sh
COPY db2catalog.sh /opt/demo/db2catalog.sh
COPY liveness.sh /liveness.sh

EXPOSE 8080
CMD ["/opt/demo/startserver.sh"]
RUN chmod -R 777 /opt/demo