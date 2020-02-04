# Track Module

This track module enables the model railway to be controlled via
a HTTP based API and an L298N module.

Supported Hardware

 * ESP32
 * L298N module

# API Reference

*API Version*: 1.0


| API URL | Function |
|---------|----------|
|/|Basic Information|
|/debug|Debug Information|
|/ping|Health Check|
|/status|Module Status|
|/api/1/tm/debug|Track Module Debug|
|/api/1/tm/hc|Track Module Health Check|
|/api/1/tm/info|Track Module Info|
|/api/1/tm/status|Track Module Status|
|/api/1/tm/profile|Track Module Profile List|
|/api/1/tm/profile/hornby|Apply Hornby Profile|
|/api/1/tm/profile/heljan|Apply Heljan Profile|
|/api/1/tm/profile/dapol|Apply Dapol Profile|
|/api/1/tm/profile/bachmann|Apply Bachmann Profile|
|/api/1/tm/profile/custom|Apply Custom Profile|
|/api/1/tm/0/speed/current|Track A Current Speed|
|/api/1/tm/1/speed/current|Track B Current Speed|
|/api/1/tm/0/speed/up|Track A Increase Speed|
|/api/1/tm/1/speed/up|Track B Increase Speed|
|/api/1/tm/0/speed/down|Track A Decrease Speed|
|/api/1/tm/1/speed/down|Track B Decrease Speed|
|/api/1/tm/0/speed/stop|Track A Slow to Stop|
|/api/1/tm/1/speed/stop|Track B Slow to Stop|
|/api/1/tm/0/stop|Track A Immediate Stop|
|/api/1/tm/1/stop|Track B Immediate Stop|
|/api/1/tm/stop|Immediate Stop - All Tracks|
|/api/1/tm/0/coast|Track A Set Speed to Normal Running Speed|
|/api/1/tm/1/coast|Track B Set Speed to Normal Running Speed|
|/api/1/tm/0/shunt|Track A Set Speed to Shunt|
|/api/1/tm/1/shunt|Track B Set Speed to Shunt|
|/api/1/tm/0/crawl|Track A Set Speed to Crawl|
|/api/1/tm/1/crawl|Track B Set Speed to Crawl|
|/api/1/tm/0/full|Track A Set Speed to Full|
|/api/1/tm/1/full|Track B Set Speed to Full|
|/api/1/tm/0/brake|Track A Push Brake|
|/api/1/tm/1/brake|Track B Push Brake|
|/api/1/tm/0/brake/on|Track A Toggle Brake On|
|/api/1/tm/1/brake/on|Track B Toggle Brake On|
|/api/1/tm/0/brake/off|Track A Taggle Brake Off|
|/api/1/tm/1/brake/off|Track B Toggle Brake Off|
|/api/1/tm/0/travel/up|Set Track A Direction of Travel to Up Line|
|/api/1/tm/1/travel/up|Set Track B Direction of Travel to Up Line|
|/api/1/tm/0/travel/down|Set Track A Direction of Travel to Down Line|
|/api/1/tm/1/travel/down|Set Track B Direction of Travel to Down Line|

