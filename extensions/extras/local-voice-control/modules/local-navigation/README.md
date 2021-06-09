# Local Navigation Module

The Alexa Auto SDK Local Navigation module extends Alexa's local search and navigation capabilities to offline use cases.

**Table of Contents:**

* [Overview](#overview)
* [Local Navigation Module SDK Dependencies](#local-navigation-module-dependencies)
* [Configuring the Local Navigation Module](#configuring-local-navigation-module)
    * [Auto SDK Engine Configuration](#engine-config)
    * [LVC App Configuration](#lvc-config)
* [Implementing the LocalSearchProvider Platform Interface](#implementing-local-search-provider)
    * [Conceptual Overview](#local-search-provider-overview)
    * [Search Request Schema](#search-request)
    * [Search Response Schema](#search-response)
    * [Lookup Request Schema](#lookup-request)
    * [Lookup Response Schema](#lookup-response)


## Overview <a id ="overview"></a>

The `Local Navigation` module voice-enables the offline search and navigation capabilities of the navigation provider running on the head unit. Using `Local Navigation` with LVC delights customers by allowing Alexa to search for and navigate to points of interest (POIs) including categories, chains, and entities when the vehicle has no connection to the internet. Customers can do the following:

* Search for POIs nearby or along the route
    * "Alexa, show me nearby Asian restaurants"
    * "Alexa, find me the nearest Costco"
    * "Alexa, find me a gas station along the way"
* Navigate to POIs
    * "Alexa, take me to Starbucks"
    * "Alexa, navigate to a nearby pharmacy"
    * "Alexa, get directions to Golden Gate Bridge"
* Query POI details
    * "Alexa, what's the address of Chase Bank in San Francisco?"
    * "Alexa, what's the phone number of Whole Foods?"

Offline search with `Local Navigation` depends on deep integration with the geo-catalog of a 3P navigation provider. When offline, Alexa fulfills customer requests by generating search queries for the navigation provider, which must return data when invoked by the implementation of the Auto SDK platform interface [`LocalSearchProvider`](#implementing-local-search-provider).


## Local Navigation Module SDK Dependencies <a id="local-navigation-module-dependencies"></a>

Using the `Local Navigation` module requires the following:

* Building and configuring the Local Voice Control (LVC) extension's [`LocalVoiceControl`](../local-voice-control/README.md) and [`LocalSkillService`](../local-skill-service/README.md) modules.
* Running the offline Alexa engine, its skills, and services that are packaged with the installation script [`LVC.sh`](../../LVC-Linux.md).
* Implementing the optional Auto SDK platform interfaces `LocationProvider`, `TemplateRuntime`, and `Navigation` in addition to all required platform interfaces.


## Configuring the Local Navigation Module <a id="configuring-local-navigation-module"></a>

Using the `Local Navigation` module requires configuring both the Auto SDK Engine and the LVC app. Local search components running in the Engine communicate via Unix Domain Sockets (UDS) with the offline Alexa service and the local navigation skill that run in the separate LVC process. Equivalent fields in each of the two configurations must match.

### Auto SDK Engine Configuration <a id="engine-config"></a>

Configure the Engine with the *"aace.localNavigation"* JSON node:

```jsonc
{
   "aace.localNavigation": {
        "localSearch": {
            "navigationPOISocketPath": "/opt/LVC/data/poi-er-service/poi_navigation.socket",
            "poiEERSocketPath": "/opt/LVC/data/poi-er-service/poi_eer.socket"
        }
    }
}
```

| Property | Type | Required | Description | Example
|-|-|-|-|-|
| aace.localNavigation.<br>localSearch.<br>navigationPOISocketPath | string | Yes | An absolute path where LVC creates a socket for communication between the local navigation skill in an LVC process and the local search components of the Auto SDK Engine. | "/opt/LVC/data/poi-er-service/poi_navigation.socket"
| aace.localNavigation.<br>localSearch.<br>poiEERSocketPath | string | Yes | An absolute path where LVC creates a socket for communication between the offline Alexa service in an LVC process and the local search components of the Auto SDK Engine. | "/opt/LVC/data/poi-er-service/poi_eer.socket"

Like all Auto SDK Engine configurations, you can either define this JSON in a file and construct an `EngineConfiguration` from that file, or you can use the provided configuration factory class [`aace::localNavigation::config::LocalNavigationConfiguration`](../local-navigation/platform/include/AACE/LocalNavigation/LocalNavigationConfiguration.h) to programmatically construct the `EngineConfiguration` in the proper format.

### LVC App Configuration <a id="lvc-config"></a>

The LVC configuration file, `lvc-config.json`, installed at `/opt/LVC/config` by the installation script, `LVC.sh`, already contains configuration for offline local search and navigation. The socket paths provided in the default configuration `lvc-config.json` match the socket paths in the sample Engine configuration above, so no additional action is required if you use these paths. For reference, the Engine configuration field *"navigationPOISocketPath"* corresponds to *"Skill.AutomotiveNavigationSkillId.POIERServiceSocketName"*, and *"poiEERSocketPath"* corresponds to the socket details of the external authority endpoint *"LOCAL_SEARCH_POI_ER_SERVICE"*.

## Implementing the LocalSearchProvider Platform Interface <a id="implementing-local-search-provider"></a>

The `LocalSearchProvider` platform interface is a search interface for the Engine to delegate offline search requests to your integration with an onboard navigation provider. Requests from the Engine are asynchronous with responses correlated by request IDs. Extend the class [`aace::localNavigation::LocalSearchProvider`](../local-navigation/platform/include/AACE/LocalNavigation/LocalSearchProvider.h), implement the virtual methods (described in detail in the following section), and register your implementation with the Engine.

```c++
#include <AACE/LocalNavigation/LocalSearchProvider.h>

class LocalSearchProviderHandler : public aace::localNavigation::LocalSearchProvider {

    public:
    bool poiSearchRequest(const std::string& request) override {
        // Return true immediately if the implementation will handle the request.
        // On a separate thread, perform the search for POIs according to the parameters of the specified request.
        // When POI matches are available, call poiSearchResponse() with the results.
        return true;
    }

    bool poiLookupRequest(const std::string& request) override {
        // Return true immediately if the implementation will handle the request.
        // On a separate thread, look up additional details about the POIs in the request.
        // When the details are available, call poiLookupResponse() with the results.
        return true;
    }
};

...

// Register the platform interface implementation with the Engine
m_engine->registerPlatformInterface(std::make_shared<LocalSearchProviderHandler>());

```

### Conceptual Overview <a id="local-search-provider-overview"></a>

Your implementation of `LocalSearchProvider` depends on the capabilities and search APIs available from the 3P navigation provider with which you integrate. The responsibility of your implementation is to translate the parameters of a generic request generated by Alexa into a request that suits your chosen provider. Alexa uses the data returned from a request to respond to a user utterance, which includes generating GUI displays of details of the requested POIs, issuing data-dependent speech, and generating further requests to the navigation provider to start navigating to one of the POIs.

It is crucial that your implementation returns accurate results that are also quick enough to avoid perceivable latency. To help with this, Alexa divides search requests for POIs into two phases: *search* and *lookup*. When the user makes an initial voice request, the Engine calls the method `LocalSearchProvider::poiSearchRequest()` and expects a response via `LocalSearchProvider::poiSearchResponse()` containing POI matches (or an error) within four seconds. Your implementation should generate a search response that includes all required fields of the response schema and any optional fields that can be retrieved in a low-latency way. After Alexa responds to the user, for example, by displaying the list of POIs or reading out data to answer a question, the Engine might ask for additional details about one or a few of the POIs returned from search by making a call to the method `LocalSearchProvider::poiLookupRequest()`. It is expected that your response via `LocalSearchProvider::poiLookupResponse()` includes additional details about the requested POI that could not be returned with search. The data for the POI in the response to lookup is a superset of the data returned with the POI in the search response.

For example, if the user requests "coffee shops in San Francisco", your implementation's response to search might not be able to include navigation distance to each of the matching POIs if getting this distance involves planning a route. If Alexa needs this data when the user follows up with "Alexa, select the second one", the Engine calls lookup with the ID of the second POI, which gives your implementation the opportunity to perform the higher-latency task of computing a route to the second coffee shop. Using the response of the lookup call, Alexa may display a detailed GUI view of this POI to the user or give a detailed answer to a user query about the POI.

### Search Request Schema <a id="search-request"></a>

The schema for search requests is as follows:

```jsonc
{
    "requestId": "{STRING}",
    "query": "{STRING}",
    "queryType": "{STRING}",
    "geolocation": [{DOUBLE}, {DOUBLE}],
    "locale": "{STRING}",
    "numOfResults": {INTEGER},
    "rankingStrategy": "{STRING}",
    "route": [[{DOUBLE}, {DOUBLE}]],
    "destination": [{DOUBLE}, {DOUBLE}],
    "searchLocation": {
        "streetAddress": "{STRING}",
        "city": "{STRING}",
        "state": "{STRING}",
        "country": "{STRING}",
        "postalCode": "{STRING}",
        "location": "{STRING}"
    }
}
```

| Property | Type | Required | Description | Example
|-|-|-|-|-|
| requestId | string | Yes | The unique identifier of this request, which correlates a request with a response. | "4da2419b-6596-4a51-9707-80fbd6960e3b"
| query | string | Yes | The search string from the user utterance. | "coffee shops",<br>"Starbucks",<br>"SFO airport"
| queryType | string | Yes | The type of search the user requests.<br><br>**Note:** This is a best approximation from Alexa NLU and may not be 100% accurate.<br><br>**Accepted values:**<ul><li>`"POI"`: Entity or chain</li><li>`"CATEGORY"`: Category</li><li>`"ADDRESS"` (unused): Address</li></ul> | "POI"
| geolocation | list\<double\> | No | The coordinate pair (latitude, longitude) to use as a bias for results of a search near the user. This value is the user location provided by your implementation of the `LocationProvider` platform interface | [37.80435, -122.27116]
| locale | string | Yes | The locale in which to return search results. <br><br>**Note:** Only "en-US" is supported.  | "en-US"
| numOfResults | integer | No | The number of search results requested by Alexa. <br><br>**Note:** If excluded, choose a value that optimizes result quality with latency according to the search provider. | 8
| rankingStrategy | string | Yes | The strategy for ranking the search results. Use the ranking strategy with the other parameters of the search request to construct a request to the navigation provider that yields results most relevant to the user request. <br><br>**Accepted values:**<ul><li>`"RELEVANCE"`: This is the default ranking strategy when the vehicle has no active navigation session. Favor results most relevant to the user query within the parameters of the search.</li><li>`"DISTANCE"`: If the `searchLocation` parameter is absent from the request, favor results closest to the user geolocation. If `searchLocation` is present, favor results closest to the location it describes.<br>This ranking strategy is used when the user explicitly requests a nearby POI or the closest POI to a specific location. </li><li>`"NEAR_DESTINATION"`: Favor results closest to the destination of the active route.<br>This ranking strategy is used when the user explicitly requests a POI near the destination.</li><li>`"ALONG_THE_ROUTE"`: Favor results closest to the active route.<br>This ranking strategy is used any time the vehicle is in an active navigation session or when the user explicitly requests a search along the route.</li></ul> | "RELEVANCE"|
| route | list\<list\<double\>\> | No | The shape of the active route as a list of coordinate pairs. This is the route shape provided in navigation state by your implementation of the `Navigation` platform interface, and it is excluded when there is no active route. <br><br>**Note:** If your implementation of `LocalSearchProvider` uses data from the same navigation provider as that for your implementation of `Navigation`, you need not use this parameter when the provider already has a reference to the active route. This parameter is convenient for along-the-route searches using search providers that do not also implement the navigation. |
| destination | list\<double\> | No | The coordinate pair (latitude, longitude) of the destination of the active route. This value is the destination provided in navigation state by your implementation of the `Navigation` platform interface, and it is excluded when there is no active route with a destination. | [37.8199, 122.4783]
| searchLocation | object | No | The location to search if specified in the user utterance (e.g., "find coffee shops *in San Francisco, California*"). If present, use the parameters of the location description to compute a bounding box to perform the search. |
| searchLocation.<br>streetAddress | string | No | Street address | "123 Main Street"
| searchLocation.<br>city | string | No | City | "San Francisco"
| searchLocation.<br>state | string |  No | State | "CA"
| searchLocation.<br>country | string | No | Country | "USA"
| searchLocation.<br>postalCode | string | No | Postal code | "94104"
| searchLocation.<br>location | string | No | Neighborhood, district, county, or region | "Financial District"


### Search Response Schema <a id="search-response"></a>

The schemas for success and error search responses are as follows:


**Success** <a id="search-response-success"></a>

```jsonc
{
    "requestId": "{STRING}",
    "status": "SUCCESS",
    "data": {
        "results": [
            {
                "type": "{STRING}",
                "score": {DOUBLE},
                "confidence": "{STRING}",
                "provider": "{STRING}",
                "id": "{STRING}",
                "address": {
                    "city": "{STRING}",
                    "addressLine1": "{STRING}",
                    "addressLine2": "{STRING}",
                    "addressLine3": "{STRING}",
                    "stateOrRegion": "{STRING}",
                    "countryCode": "{STRING}",
                    "districtOrCounty": "{STRING}",
                    "postalCode": "{STRING}"
                },
                "navigationPosition": [{DOUBLE}, {DOUBLE}],
                "routingInfo": {
                    "travelDistance": {
                        "straightLineDistanceInMeters": {DOUBLE},
                        "navigationDistanceInMeters": {DOUBLE}
                    },
                    "travelTime": {
                        "ideal": "{STRING}",
                        "predicted": "{STRING}"
                    }
                },
                "poi": {
                    "name": "{STRING}",
                    "categories": ["{STRING}"],
                    "phoneNumber": "{STRING}",
                    "website": "{STRING}",
                    "rating": {DOUBLE},
                    "hoursOfOperation": [
                        {
                            "dayOfWeek": "{STRING}",
                            "hours": [
                                {
                                    "open": "{STRING}",
                                    "close": "{STRING}"
                                }
                            ],
                            "type": "{STRING}"
                        }
                    ],
                    "imageFilePath": "{STRING}"
                }
            }
        ],
        "totalNumResults": {INTEGER},
        "appliedRankingStrategy": "{STRING}"
    }
}
```

| Property | Type | Required | Description | Example
|-|-|-|-|-|
| requestId | string | Yes | The unique identifier of this response. Must match the `requestId` of the request to which it responds. | "4da2419b-6596-4a51-9707-80fbd6960e3b"
| status | string | Yes | The status of the search request. <br><br>**Accepted values:**<ul><li>`"SUCCESS"`</li></ul> | "SUCCESS"
| data | object | Yes | The payload containing the results of the search. |
| data.<br>totalNumResults | integer | Yes | The number of search results included in the response. May be different from the requested number of results. | 8
| data.<br>appliedRankingStrategy | string | No | The ranking strategy applied for the search. May be different from the requested ranking strategy. <br><br>**Accepted values:**<ul><li>`"RELEVANCE"`</li><li>`"DISTANCE"`</li><li>`"NEAR_DESTINATION"`</li><li>`"ALONG_THE_ROUTE"`</li></ul> | "RELEVANCE"
| data.<br>results | list | Yes | The list of matched items. |
| data.<br>results[i].<br>type | string | No | The type of the item. <br><br>**Accepted values:**<ul><li>`"ENTITY"`</li><li>`"CATEGORY"`</li><li>`"CHAIN"`</li><li>`"ADDRESS"`</li></ul> | "ENTITY"
| data.<br>results[i].<br>score | double | Yes | The score (0.0 - 100.0) for how closely the result matches the search query. | 100.0
| data.<br>results[i].<br>confidence | string | Yes | The confidence level of how relevant the result is to the search query. <br><br>**Accepted values:**<ul><li>`"HIGH"`</li><li>`"MEDIUM"`</li><li>`"LOW"`</li></ul> | "HIGH"
| data.<br>results[i].<br>provider | string | Yes | The provider of the search result. | "TomTom",<br>"HERE",<br>"Telenav"
| data.<br>results[i].<br>id | string | Yes | A unique identifier of the item. Alexa uses the ID of the item to perform subsequent lookup requests for further details about the item. | "4112"
| data.<br>results[i].<br>address | object | No | The address details of the item. |
| data.<br>results[i].<br>address.<br>city | string | No | The city of the item. | "San Francisco"
| data.<br>results[i].<br>address.<br>addressLine1 | string | No | The first line of the item's address. | "601 Van Ness Ave"
| data.<br>results[i].<br>address.<br>addressLine2 | string | No | The second line of the item's address. |
| data.<br>results[i].<br>address.<br>addressLine3 | string | No | The third line of the item's address. |
| data.<br>results[i].<br>address.<br>stateOrRegion | string | No | The state or region of the item. | "CA"
| data.<br>results[i].<br>address.<br>countryCode | string | No | The 3-letter country code (ISO 3166-1 alpha-3 format) of the item's country. | "USA"
| data.<br>results[i].<br>address.<br>districtOrCounty | string | No | The district or country of the item. | "Financial District"
| data.<br>results[i].<br>address.<br>postalCode | string | No | The postal code of the item. | "94102"
| data.<br>results[i].<br>navigationPosition | list\<double\> | Yes | The coordinate pair (latitude, longitude) of the item location. | [37.78139, -122.42141]
| data.<br>results[i].<br>routingInfo | object | Yes | Travel distance and time to the item. |
| data.<br>results[i].<br>routingInfo.<br>travelDistance | object | Yes | Travel distance to the item. |
| data.<br>results[i].<br>routingInfo.<br>travelDistance.<br>straightLineDistanceInMeters | double | Yes | The straight line distance to the item. | 13476.75
| data.<br>results[i].<br>routingInfo.<br>travelDistance.<br>navigationDistanceInMeters | double | No | The actual travel distance to the item according to a planned route. | 19297.12
| data.<br>results[i].<br>routingInfo.<br>travelTime | object | No | The estimated arrival time at the item. |
| data.<br>results[i].<br>routingInfo.<br>travelTime.<br>ideal | String | No | The expected arrival time (ISO-8601 time format) at the item without considering traffic, diversions, and other factors. | "2020-12-07T10:06:42.500-0800"
| data.<br>results[i].<br>routingInfo.<br>travelTime.<br>predicted | String | Yes if `travelTime` is present | The predicted arrival time (ISO-8601 time format) at the item considering traffic and other factors. | "2020-12-07T10:06:49.400-0800"
| data.<br>results[i].<br>poi | object | Yes if the item is a POI, no otherwise | Additional details about the point of interest. |
| data.<br>results[i].<br>poi.<br>name | string | Yes | The name of the POI. | "Peet's Coffee & Tea"
| data.<br>results[i].<br>poi.<br>categories | list<string> | No | The categories associated with the POI. | ["coffee shop", "cafe"]
| data.<br>results[i].<br>poi.<br>phoneNumber | string | No | The phone number (E.164 format) of the POI. | "+12223334444"
| data.<br>results[i].<br>poi.<br>website | string | No | The web address of the POI. | "https://www.peets.com"
| data.<br>results[i].<br>poi.<br>rating | double | No | The rating (1-5) of the POI. | 4.6
| data.<br>results[i].<br>poi.<br>hoursOfOperation | list | No | The hours of operation for the POI. |
| data.<br>results[i].<br>poi.<br>hoursOfOperation[j] | object | Yes if `hoursOfOperation` is present | Hours of operation on a weekday. |
| data.<br>results[i].<br>poi.<br>hoursOfOperation[j].<br>.dayOfWeek | string | Yes | The weekday represented by this object. <br><br>**Accepted values:**<ul><li>`"MONDAY"`</li><li>`"TUESDAY"`</li><li>`"WEDNESDAY"`</li><li>`"THURSDAY"`</li><li>`"FRIDAY"`</li><li>`"SATURDAY"`</li><li>`"SUNDAY"`</li></ul> | "MONDAY"
| data.<br>results[i].<br>poi.<br>hoursOfOperation[j].<br>.hours | list | Yes | The list of opening and closing hours for the day. For example, the POI might open in the morning, close for lunch, and reopen later in the day and hence has two hour ranges for the day. |
| data.<br>results[i].<br>poi.<br>hoursOfOperation[j].<br>hours[k] | object | Yes | A single range of opening and closing hours for the day. <br><br> `open` and `close` hours are ISO-8601 time:<br>*time-hour*:*time-minute*:*time-second*±*time-zone-offset*| ``` { "open": "12:30:00-8:00", "close": "1:00:00-8:00"}```
| data.<br>results[i].<br>poi.<br>hoursOfOperation[j].<br>type | string | Yes | The description of the hours of operation for the day. <br><br>**Accepted values:**<ul><li>`"OPEN_DURING_HOURS"`: The POI is only open during the specified hours</li><li>`"OPEN_24_HOURS"`: The POI is open 24 hours</li><li>`"CLOSED"`: The POI is closed</li><li>`"HOLIDAY"`: The POI is closed due to a holiday</li><li>`"UNKNOWN"`: The information is not available for the POI</li></ul> | "OPEN_DURING_HOURS"
| data.<br>results[i].<br>poi.<br>imageFilePath | string | No | The path on the device for an image for the POI. |


**Error** <a id="search-response-error"></a>

```jsonc
{
    "requestId": "{STRING}",
    "status": "FAIL",
    "error": {
        "errorCode": "{STRING}",
        "errorMessage": "{STRING}"
    }
}
```
| Property | Type | Required | Description | Example
|-|-|-|-|-|
| requestId | string | Yes | The unique identifier of this response. Must match the `requestId` of the request to which it responds. | "4da2419b-6596-4a51-9707-80fbd6960e3b"
| status | string | Yes | The status of the search request. <br><br>**Accepted values:**<ul><li>`"FAIL"`</li></ul> | "FAIL"
| error.<br>errorCode | string | Yes | The type of error. <br><br>**Accepted values:**<ul><li>`"INTERNAL_ERROR"`</li><li>`"SUBSCRIPTION_EXPIRED"`</li></ul>  | "INTERNAL_ERROR"
| error.<br>errorMessage | string | No | A short description of the error.  | "Request timed out"


### Lookup Request Schema <a id="lookup-request"></a>

The schema for lookup requests is as follows:


```jsonc
{
    "requestId": "{STRING}",
    "locale": "{STRING}",
    "lookupIds": ["{STRING}"]
}
```

| Property | Type | Required | Description | Example
|-|-|-|-|-|
| requestId | string | Yes | The unique identifier of this request, which correlates a request with a response. | "alexahybrid.ask.request.8a81089a-4ab9-4336-b31a-de92b28317e5"
| locale | string | Yes | The locale in which to return search results. <br><br>**Note:** Only "en-US" is supported.  | "en-US"
| lookupIds | list<string> | Yes | A list of IDs that Alexa looks up for additional details. Theses IDs match the ones returned in your prior response to search. <br><br>**Note:** The list contains one ID. | ["4112"]


### Lookup Response Schema <a id="lookup-response"></a>

The schemas for success and error lookup responses are as follows:

**Success** <a id="lookup-response-success"></a>

The lookup success response schema is the same as the search success response schema.

**Error** <a id="lookup-response-error"></a>

The lookup error response schema is the same as the search error response schema.
