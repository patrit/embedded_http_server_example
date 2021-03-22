# Example HTTP Server for Migrating CICS Applications

## Development Environment

### C/C++ development environment
Use any well known compiler e.g. gcc/clang.
This example uses Poco as embedded HTTP server (https://pocoproject.org/).

### Database DB2
As CICS is running on the Mainframe, DB2 is the default database. The development will be done with DB2 (formerly known as DB2 LUW).

A running database is required to build the DB2 packages.

The development should be done against a local database that is independent of any existing environment to reduce conflicts during development.

When using embedded SQL, each commit should generate new packages with it's own version to support rolling updates. Proposal: use the git commit hash to generate a version. The consistency token has to be bound to the version, default is the timestamp, because otherwise new build of the application for a given git hash fails to be used against the DB2 without prior bind commands.

Analyzing bind files for version matching:
```bash
for i in $(find . -name "*.bnd"); do db2bfd -b $i; done
```

### Download DB2 Stuff
Atm version 11.5
- DB2 server: https://www.ibm.com/analytics/db2/trials
- DB2 runtime client: https://epwt-www.mybluemix.net/software/support/trial/cst/programwebsite.wss?siteId=848&h=null&tabId=#

```bash
md5sum baseimages/*/*.tar.gz
eba5b16dd24f6c253667c7cb1aebf262  baseimages/db2/v11.5.5_linuxx64_server_dec.tar.gz
4683947ef9772698c7047a2f2fe0f970  baseimages/db2client/v11.5.4_linuxx64_rtcl.tar.gz
```

### VM

Install DB2 and development environment according to Dockerfile commands

```bash
# setup DB2 reference - replace port accordingly if required
db2 catalog tcpip node db2local remote localhost server 50000
db2 catalog database SAMPLE as SAMPLE at node db2local
db2 terminate
```

### Docker

Base images are used to reprevent rebuilding the whole container for every build.
```bash
VERSION=0.1
(cd baseimages/db2 && docker build -t db2:$VERSION .)
(cd baseimages/dev && docker build --build-arg VERSION=$VERSION -t dev:$VERSION .)
(cd baseimages/db2client && docker build -t db2client:$VERSION .)

$ docker images | grep $VERSION
dev                 0.1                 270592add272        13 minutes ago      4.42GB
db2client           0.1                 9efeba185f72        2 hours ago         1.23GB
db2                 0.1                 b00f94060567        2 days ago          3.59GB
```
Note:
- Image hashes may differ
- The image sizes remain pretty big - also for the runtime, due to copied archive
- grabbing the archives from an internal HTTP server while installing can eliminate this issue

## Build

### ... with VM

```bash
# supposing a running DB2 server with a SAMPLE database
make -j4 all
# Note: the packages are bound locally with the build step
# start the server
DBNAME=SAMPLE DBA=dude DBA_PASSWD=pw42 ./server -p 8080
```

### ... with containers

```bash
# preferrably done in a build pipeline

# once or for each build depending on the setup
docker network create -d bridge myNetwork
docker run -d --name mydb2 --network=myNetwork --network-alias=db2 --privileged=true -p 50000:50000 -e LICENSE=accept -e DB2INSTANCE=dude -e DB2INST1_PASSWORD=pw42 -e DBNAME=testdb -e SAMPLEDB=true ibmcom/db2:11.5.5.0

# repeating for each build
docker run --rm --privileged -v $(pwd):/server --network=myNetwork -e TZ=CET -e LOCAL_USER_ID=$(id -u) dev /runcmd.sh bash -c "cd /server && DB2_HOST=db2 make -j4 all"

# building containers for deployment
(rm -rf docker/bindsql/bind && mkdir -p docker/bindsql/bind && find . -name "*.bnd" | xargs cp -t docker/bindsql/bind/ && cd docker/bindsql && docker build -t bindsql .)
(cp -f server docker/server/server && cd docker/server && docker build -t demoserver .)

# deploy - here locally wih docker
docker run --rm --name mydb2bind --network=myNetwork bindsql
docker run --rm --name myserver --network=myNetwork -p8080:8080 demoserver
```


## Test

### ... functional

```bash
curl -vvv http://localhost:8080/api -d "<dummydata/>" -H 'Accept: application/xml, text/plain, */*' -H 'Content-Type: application/xml'

*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to localhost (127.0.0.1) port 8080 (#0)
> POST /api HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.58.0
> Accept: application/xml, text/plain, */*
> Content-Type: application/xml
> Content-Length: 12
> 
* upload completely sent off: 12 out of 12 bytes
< HTTP/1.1 200 OK
< Date: Sun, 21 Mar 2021 18:32:24 GMT
< Connection: Close
< Content-Type: text/xml
< 
BRUCE,ADAMSON,16
ELIZABETH,PIANKA,17
MASATOSHI,YOSHIMURA,16
MARILYN,SCOUTTEN,17
JAMES,WALKER,16
DAVID,BROWN,16
WILLIAM,JONES,17
JENNIFER,LUTZ,18
KIYOSHI,YAMAMOTO,16
REBA,JOHN,18
REBA,JOHN,18
* Closing connection 0
```

### ... performance

```bash
echo "<dummyxml/>" > data.xml
ab -n 1000 -p data.xml -T "application/xml" http://localhost:8080/api

This is ApacheBench, Version 2.3 <$Revision: 1807734 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient).....done


Server Software:
Server Hostname:        localhost
Server Port:            8080

Document Path:          /api
Document Length:        191 bytes

Concurrency Level:      1
Time taken for tests:   1.216 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      290000 bytes
Total body sent:        149000
HTML transferred:       191000 bytes
Requests per second:    822.03 [#/sec] (mean)
Time per request:       1.216 [ms] (mean)
Time per request:       1.216 [ms] (mean, across all concurrent requests)
Transfer rate:          232.80 [Kbytes/sec] received
                        119.61 kb/s sent
                        352.41 kb/s total

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       0
Processing:     1    1   1.4      1      31
Waiting:        1    1   1.4      1      31
Total:          1    1   1.4      1      31

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      2
  98%      4
  99%      6
 100%     31 (longest request)
```
