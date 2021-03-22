FROM ubuntu:18.04

## Name of the downloaded file.
ENV DB2_INSTALLER v11.5.4_linuxx64_rtcl.tar.gz
# Directory of the installers. Associated to the edition.
ENV DB2_INST_DIR rtcl
## Version of the downloaded file. Used for db2prereqcheck.
ENV DB2_VERSION 11.5.4.0
## Name of the response file included in the Docker image.
ENV DB2_RESP_FILE db2rtcl.rsp
## Directory where DB2 is installed.
ENV DB2_DIR /opt/ibm/db2
## Add DB2 response file
COPY ${DB2_RESP_FILE} /tmp/
COPY ${DB2_INSTALLER} /tmp/

RUN  apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    tzdata locales \
    aria2 libnuma1 \
    unzip \
    libaio1 \
    binutils \    
    netbase \
    libxml2 libstdc++6 libpoconet50 libpocoutil50

RUN cd /tmp && \
    # <curl archive from server> && \
    tar xvfz ${DB2_INSTALLER} && \
    rm -f ${DB2_INSTALLER} && \
    cd ${DB2_INST_DIR} && \
    ./db2prereqcheck && \
    ( ./db2setup -r /tmp/${DB2_RESP_FILE} && \
    cat /tmp/db2setup.log || cat /tmp/db2setup.log ) && \
    cd ${DB2_DIR} && \
    rm -rf Readme lib32 java json tools desktop install secutity32 && \
    cd /tmp && rm -Rf * && \
    apt-get autoclean -y && \
    apt-get clean -y

RUN chmod -R 777 /home/db2admin/
