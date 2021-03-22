FROM ubuntu:18.04

## Name of the downloaded file.
ENV DB2_INSTALLER v11.5.5_linuxx64_server_dec.tar.gz
## Directory of the installers. Associated to the edition.
ENV DB2_INST_DIR server_dec
## Name of the response file included in the Docker image.
ENV DB2_RESP_FILE db2expc.rsp
## Directory where DB2 is installed.
ENV DB2_DIR /opt/ibm/db2
## DB2 Instance.
ENV DB2INSTANCE dude
## Add DB2 response file
COPY ${DB2_RESP_FILE} /tmp/${DB2_RESP_FILE}
COPY ${DB2_INSTALLER} /tmp/${DB2_INSTALLER}

# Updates Linux. Includes i386
RUN dpkg --add-architecture i386 && \
  apt-get update && apt-get upgrade -y && \
  apt-get install -y \
    aria2 \
    curl \
    unzip \
    libaio1 \
    binutils \
    netbase \
    libpam-ldap:i386 \
    libstdc++6 \
    lib32stdc++6 \
    libnuma1 && \
  cd /tmp && \
  # <curl archive from server> && \
  tar xfz ${DB2_INSTALLER} && \
  rm -f ${DB2_INSTALLER} && \
  cd ${DB2_INST_DIR} && \
  ./db2prereqcheck && \
  ( ./db2setup -r /tmp/${DB2_RESP_FILE} && \
  cat /tmp/db2setup.log || cat /tmp/db2setup.log ) && \
  ${DB2_DIR}/bin/db2val -o && \
  cd && \
  rm -Rf /tmp/${DB2_INST_DIR} && \
  echo "0 localhost 0" >/home/db2admin/sqllib/db2nodes.cfg

# Removes the cache of apt-get and the unnecessary packages.
RUN apt-get purge -y aria2 curl && \
  apt-get autoclean -y && \
  apt-get clean -y
