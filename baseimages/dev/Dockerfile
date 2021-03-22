ARG VERSION
FROM db2:$VERSION

RUN apt-get update && apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
      make git \
      binutils-dev \
      libc6-dev libssl-dev \
      clang clang-format clang-tools clang-tidy libpoco-dev curl zip && \
    apt-get autoclean -y && \
    apt-get clean -y

COPY catalogdb.sh  /catalogdb.sh
COPY runcmd.sh  /runcmd.sh

ENV LD_LIBRARY_PATH /opt/ibm/db2/lib64
ENV PATH=/opt/ibm/db2/bin/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

CMD ["/catalogdb.sh && bash"]


