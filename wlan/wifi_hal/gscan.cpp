//#define LOG_NDEBUG 0
#include <stdint.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <linux/rtnetlink.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#include <linux/errqueue.h>

#include <linux/pkt_sched.h>
#include <netlink/object-api.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/handlers.h>

#include "sync.h"

#define LOG_TAG  "WifiHAL"

#include <utils/Log.h>

#include "wifi_hal.h"
#include "common.h"
#include "cpp_bindings.h"

typedef enum {
    NL80211_ATTR_VENDOR_CAPABILITIES = 1,

    GSCAN_ATTRIBUTE_NUM_BUCKETS = 10,
    GSCAN_ATTRIBUTE_BASE_PERIOD,
    GSCAN_ATTRIBUTE_BUCKETS_BAND,
    GSCAN_ATTRIBUTE_BUCKET_ID,
    GSCAN_ATTRIBUTE_BUCKET_PERIOD,
    GSCAN_ATTRIBUTE_BUCKET_NUM_CHANNELS,
    GSCAN_ATTRIBUTE_BUCKET_CHANNELS,
    GSCAN_ATTRIBUTE_NUM_AP_PER_SCAN,
    GSCAN_ATTRIBUTE_REPORT_THRESHOLD,
    GSCAN_ATTRIBUTE_NUM_SCANS_TO_CACHE,
    GSCAN_ATTRIBUTE_BAND = GSCAN_ATTRIBUTE_BUCKETS_BAND,

    GSCAN_ATTRIBUTE_ENABLE_FEATURE = 20,
    GSCAN_ATTRIBUTE_SCAN_RESULTS_COMPLETE,	      /* indicates no more results */
    GSCAN_ATTRIBUTE_FLUSH_FEATURE,		      /* Flush all the configs */
    GSCAN_ENABLE_FULL_SCAN_RESULTS,
    GSCAN_ATTRIBUTE_REPORT_EVENTS,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_NUM_OF_RESULTS = 30,
    GSCAN_ATTRIBUTE_FLUSH_RESULTS,
    GSCAN_ATTRIBUTE_SCAN_RESULTS,		       /* flat array of wifi_scan_result */
    GSCAN_ATTRIBUTE_SCAN_ID,			    /* indicates scan number */
    GSCAN_ATTRIBUTE_SCAN_FLAGS,			 /* indicates if scan was aborted */
    GSCAN_ATTRIBUTE_AP_FLAGS,			   /* flags on significant change event */
    GSCAN_ATTRIBUTE_NUM_CHANNELS,
    GSCAN_ATTRIBUTE_CHANNEL_LIST,
    GSCAN_ATTRIBUTE_CH_BUCKET_BITMASK,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_SSID = 40,
    GSCAN_ATTRIBUTE_BSSID,
    GSCAN_ATTRIBUTE_CHANNEL,
    GSCAN_ATTRIBUTE_RSSI,
    GSCAN_ATTRIBUTE_TIMESTAMP,
    GSCAN_ATTRIBUTE_RTT,
    GSCAN_ATTRIBUTE_RTTSD,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_HOTLIST_BSSIDS = 50,
    GSCAN_ATTRIBUTE_RSSI_LOW,
    GSCAN_ATTRIBUTE_RSSI_HIGH,
    GSCAN_ATTRIBUTE_HOTLIST_ELEM,
    GSCAN_ATTRIBUTE_HOTLIST_FLUSH,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_RSSI_SAMPLE_SIZE = 60,
    GSCAN_ATTRIBUTE_LOST_AP_SAMPLE_SIZE,
    GSCAN_ATTRIBUTE_MIN_BREACHING,
    GSCAN_ATTRIBUTE_NUM_AP,
    GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_BSSIDS,
    GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_FLUSH,

    /* EPNO */
    GSCAN_ATTRIBUTE_EPNO_SSID_LIST = 70,
    GSCAN_ATTRIBUTE_EPNO_SSID,
    GSCAN_ATTRIBUTE_EPNO_SSID_LEN,
    GSCAN_ATTRIBUTE_EPNO_RSSI,
    GSCAN_ATTRIBUTE_EPNO_FLAGS,
    GSCAN_ATTRIBUTE_EPNO_AUTH,
    GSCAN_ATTRIBUTE_EPNO_SSID_NUM,
    GSCAN_ATTRIBUTE_EPNO_FLUSH,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_WHITELIST_SSID = 80,
    GSCAN_ATTRIBUTE_NUM_WL_SSID,
    GSCAN_ATTRIBUTE_WL_SSID_LEN,
    GSCAN_ATTRIBUTE_WL_SSID_FLUSH,
    GSCAN_ATTRIBUTE_WHITELIST_SSID_ELEM,
    GSCAN_ATTRIBUTE_NUM_BSSID,
    GSCAN_ATTRIBUTE_BSSID_PREF_LIST,
    GSCAN_ATTRIBUTE_BSSID_PREF_FLUSH,
    GSCAN_ATTRIBUTE_BSSID_PREF,
    GSCAN_ATTRIBUTE_RSSI_MODIFIER,
    /* remaining reserved for additional attributes */

    GSCAN_ATTRIBUTE_A_BAND_BOOST_THRESHOLD = 90,
    GSCAN_ATTRIBUTE_A_BAND_PENALTY_THRESHOLD,
    GSCAN_ATTRIBUTE_A_BAND_BOOST_FACTOR,
    GSCAN_ATTRIBUTE_A_BAND_PENALTY_FACTOR,
    GSCAN_ATTRIBUTE_A_BAND_MAX_BOOST,
    GSCAN_ATTRIBUTE_LAZY_ROAM_HYSTERESIS,
    GSCAN_ATTRIBUTE_ALERT_ROAM_RSSI_TRIGGER,
    GSCAN_ATTRIBUTE_LAZY_ROAM_ENABLE,

    /* BSSID blacklist */
    GSCAN_ATTRIBUTE_BSSID_BLACKLIST_FLUSH = 100,
    GSCAN_ATTRIBUTE_BLACKLIST_BSSID,

    /* ANQPO */
    GSCAN_ATTRIBUTE_ANQPO_HS_LIST = 110,
    GSCAN_ATTRIBUTE_ANQPO_HS_LIST_SIZE,
    GSCAN_ATTRIBUTE_ANQPO_HS_NETWORK_ID,
    GSCAN_ATTRIBUTE_ANQPO_HS_NAI_REALM,
    GSCAN_ATTRIBUTE_ANQPO_HS_ROAM_CONSORTIUM_ID,
    GSCAN_ATTRIBUTE_ANQPO_HS_PLMN,

    /* Adaptive scan attributes */
    GSCAN_ATTRIBUTE_BUCKET_STEP_COUNT = 120,
    GSCAN_ATTRIBUTE_BUCKET_MAX_PERIOD,

    /* ePNO cfg */
    GSCAN_ATTRIBUTE_EPNO_5G_RSSI_THR = 130,
    GSCAN_ATTRIBUTE_EPNO_2G_RSSI_THR,
    GSCAN_ATTRIBUTE_EPNO_INIT_SCORE_MAX,
    GSCAN_ATTRIBUTE_EPNO_CUR_CONN_BONUS,
    GSCAN_ATTRIBUTE_EPNO_SAME_NETWORK_BONUS,
    GSCAN_ATTRIBUTE_EPNO_SECURE_BONUS,
    GSCAN_ATTRIBUTE_EPNO_5G_BONUS,

    GSCAN_ATTRIBUTE_MAX

} GSCAN_ATTRIBUTE;

void convert_to_hal_result(wifi_scan_result *to, wifi_gscan_result_t *from)
{
    to->ts = from->ts;
    to->channel = from->channel;
    to->rssi = from->rssi;
    to->rtt = from->rtt;
    to->rtt_sd = from->rtt_sd;
    to->beacon_period = from->beacon_period;
    to->capability = from->capability;
    memcpy(to->ssid, from->ssid, (DOT11_MAX_SSID_LEN+1));
    memcpy(&to->bssid, &from->bssid, sizeof(mac_addr));
}

/////////////////////////////////////////////////////////////////////////////
// Mtk wifi vendor data format
typedef struct {
    int max_scan_cache_size;		// total space allocated for scan (in bytes)
    int max_scan_buckets;		// maximum number of channel buckets
    int max_ap_cache_per_scan;		// maximum number of APs that can be stored per scan
    int max_rssi_sample_size;		// number of RSSI samples used for averaging RSSI
    int max_scan_reporting_threshold;	// max possible report_threshold as described
					// in wifi_scan_cmd_params
    int max_hotlist_aps;
    int max_significant_wifi_change_aps;// maximum number of entries for
					// significant wifi change APs
    int max_bssid_history_entries;	// number of BSSID/RSSI entries that device can hold
} mtk_wifi_gscan_cap;

class GetCapabilitiesCommand : public WifiCommand
{
    wifi_gscan_capabilities *mCapabilities;
public:
    GetCapabilitiesCommand(wifi_interface_handle iface, wifi_gscan_capabilities *capabitlites)
	: WifiCommand(iface, 0), mCapabilities(capabitlites)
    {
	memset(mCapabilities, 0, sizeof(*mCapabilities));
    }

    virtual int create() {
	ALOGD("[WIFI HAL]Creating message to get gscan capablities; handle=%p, iface=%d, ifname=%s", 
			mIfaceInfo->handle, mIfaceInfo->id, mIfaceInfo->name);
		
	int ret = mMsg.create(GOOGLE_OUI, GSCAN_SUBCMD_GET_CAPABILITIES);
	if (ret < 0) {
	    return ret;
	}

	return ret;
    }

protected:
    virtual int handleResponse(WifiEvent& reply) {

	ALOGD("In GetCapabilities::handleResponse");

	if (reply.get_cmd() != NL80211_CMD_VENDOR) {
	    ALOGD("Ignoring reply with cmd = %d", reply.get_cmd());
	    return NL_SKIP;
	}

	int id = reply.get_vendor_id();
	int subcmd = reply.get_vendor_subcmd();
	int wiphy_id = reply.get_u32(NL80211_ATTR_WIPHY);
	int if_id = reply.get_u32(NL80211_ATTR_IFINDEX);

	struct nlattr *vendor_data = (struct nlattr *)reply.get_vendor_data();
	int len = reply.get_vendor_data_len();
	void *payload = NULL;
	if(vendor_data->nla_type == NL80211_ATTR_VENDOR_CAPABILITIES)
	{
	    payload = nla_data(vendor_data);
	    len -= NLA_HDRLEN;
	}

	ALOGD("wiphy_id=%d, if_id=%d, Id = %0x, subcmd = %d, len = %d, expected len = %d, payload=%p",
		wiphy_id, if_id, id, subcmd, len, sizeof(*mCapabilities), payload);
	if(payload) {
	    if (len == sizeof(wifi_gscan_capabilities))		// same version
		memcpy(mCapabilities, payload, sizeof(*mCapabilities));
	    else if (len < sizeof(wifi_gscan_capabilities)) {	// old version
		mtk_wifi_gscan_cap *mtk_cap = (mtk_wifi_gscan_cap*)payload;
		mCapabilities->max_scan_cache_size		= mtk_cap->max_scan_cache_size;
		mCapabilities->max_scan_buckets			= mtk_cap->max_scan_buckets;
		mCapabilities->max_ap_cache_per_scan		= mtk_cap->max_ap_cache_per_scan;
		mCapabilities->max_rssi_sample_size		= mtk_cap->max_rssi_sample_size;
		mCapabilities->max_scan_reporting_threshold	= mtk_cap->max_scan_reporting_threshold;
		mCapabilities->max_hotlist_bssids		= mtk_cap->max_hotlist_aps; // maximum number of entries for hotlist BSSIDs
		mCapabilities->max_hotlist_ssids		= mtk_cap->max_hotlist_aps; // maximum number of entries for hotlist SSIDs
		mCapabilities->max_significant_wifi_change_aps	= mtk_cap->max_significant_wifi_change_aps;
		mCapabilities->max_bssid_history_entries	= mtk_cap->max_bssid_history_entries;
		//mCapabilities->max_number_epno_networks = 2;		// max number of epno entries
		//mCapabilities->max_number_epno_networks_by_ssid = 2;	// max number of epno entries if ssid is specified,
									// that is, epno entries for which an exact match is
									// required, or entries corresponding to hidden ssids
		//mCapabilities->max_number_of_white_listed_ssid = 4;	// max number of white listed SSIDs, M target is 2 to 4
	    }
	}
	ALOGV("max_scan_cache_size=%d, %d, %d, %d, %d, %d, %d, %d, max_bssid_history_entries=%d",
		mCapabilities->max_scan_cache_size,
		mCapabilities->max_scan_buckets,
		mCapabilities->max_ap_cache_per_scan,
		mCapabilities->max_rssi_sample_size,
		mCapabilities->max_scan_reporting_threshold,
		mCapabilities->max_hotlist_bssids,
		mCapabilities->max_hotlist_ssids,
		mCapabilities->max_significant_wifi_change_aps,
		mCapabilities->max_bssid_history_entries);
	return NL_OK;
    }
};

wifi_error wifi_get_gscan_capabilities(wifi_interface_handle handle,
	wifi_gscan_capabilities *capabilities)
{
    GetCapabilitiesCommand command(handle, capabilities);
    return (wifi_error) command.requestResponse();
}

class GetChannelListCommand : public WifiCommand
{
    wifi_channel *channels;
    int max_channels;
    int *num_channels;
    int band;
public:
    GetChannelListCommand(wifi_interface_handle iface, wifi_channel *channel_buf, int *ch_num,
	int num_max_ch, int band)
	: WifiCommand(iface, 0), channels(channel_buf),
	max_channels(num_max_ch), num_channels(ch_num), band(band)
    {
	memset(channels, 0, sizeof(wifi_channel) * max_channels);
    }
    virtual int create() {
	ALOGD("[WIFI HAL]Creating message to get channel list; iface = %d,max_channels=%d,band=%d",
		mIfaceInfo->id, max_channels, band);

	int ret = mMsg.create(GOOGLE_OUI, GSCAN_SUBCMD_GET_CHANNEL_LIST);
	if (ret < 0) {
	    return ret;
	}

	nlattr *data = mMsg.attr_start(NL80211_ATTR_VENDOR_DATA);
	ret = mMsg.put_u32(GSCAN_ATTRIBUTE_BAND, band);
	if (ret < 0) {
	    return ret;
	}

	mMsg.attr_end(data);

	return ret;
    }

protected:
    virtual int handleResponse(WifiEvent& reply) {

	ALOGD("[WIFI HAL]In GetChannelList::handleResponse");

	if (reply.get_cmd() != NL80211_CMD_VENDOR) {
	    ALOGD("Ignoring reply with cmd = %d", reply.get_cmd());
	    return NL_SKIP;
	}

	int id = reply.get_vendor_id();
	int subcmd = reply.get_vendor_subcmd();
	int num_channels_to_copy = 0;

	nlattr *vendor = reply.get_attribute(NL80211_ATTR_VENDOR_DATA);
	int len = reply.get_vendor_data_len();

	ALOGD("Id = %0x, subcmd = %d, len = %d", id, subcmd, len);
	if (vendor == NULL || len == 0) {
	    ALOGE("no vendor data in GetChannelList response; ignoring it");
	    return NL_SKIP;
	}

	for (nl_iterator it(vendor); it.has_next(); it.next()) {
	    if (it.get_type() == GSCAN_ATTRIBUTE_NUM_CHANNELS) {
		num_channels_to_copy = it.get_u32();
		ALOGI("Got channel list with %d channels", num_channels_to_copy);
		if(num_channels_to_copy > max_channels)
		    num_channels_to_copy = max_channels;
		*num_channels = num_channels_to_copy;
	    } else if (it.get_type() == GSCAN_ATTRIBUTE_CHANNEL_LIST && num_channels_to_copy) {
		memcpy(channels, it.get_data(), sizeof(int) * num_channels_to_copy);
		if(channels) {
		    ALOGV("num_channels=%d, channel[0]=%d", num_channels_to_copy, channels[0]);
		}
	    } else {
		ALOGW("Ignoring invalid attribute type = %d, size = %d",
			it.get_type(), it.get_len());
	    }
	}
	return NL_OK;
    }
};

wifi_error wifi_get_valid_channels(wifi_interface_handle handle,
	int band, int max_channels, wifi_channel *channels, int *num_channels)
{
    GetChannelListCommand command(handle, channels, num_channels,
					max_channels, band);
    return (wifi_error) command.requestResponse();
}
/////////////////////////////////////////////////////////////////////////////

/* helper functions */

static int parseScanResults(wifi_scan_result *results, int num, nlattr *attr)
{
    memset(results, 0, sizeof(wifi_scan_result) * num);

    int i = 0;
    for (nl_iterator it(attr); it.has_next() && i < num; it.next(), i++) {

	int index = it.get_type();
	ALOGI("retrieved scan result %d", index);
	nlattr *sc_data = (nlattr *) it.get_data();
	wifi_scan_result *result = results + i;

	for (nl_iterator it2(sc_data); it2.has_next(); it2.next()) {
	    int type = it2.get_type();
	    if (type == GSCAN_ATTRIBUTE_SSID) {
		strncpy(result->ssid, (char *) it2.get_data(), it2.get_len());
		result->ssid[it2.get_len()] = 0;
	    } else if (type == GSCAN_ATTRIBUTE_BSSID) {
		memcpy(result->bssid, (byte *) it2.get_data(), sizeof(mac_addr));
	    } else if (type == GSCAN_ATTRIBUTE_TIMESTAMP) {
		result->ts = it2.get_u64();
	    } else if (type == GSCAN_ATTRIBUTE_CHANNEL) {
		result->ts = it2.get_u16();
	    } else if (type == GSCAN_ATTRIBUTE_RSSI) {
		result->rssi = it2.get_u8();
	    } else if (type == GSCAN_ATTRIBUTE_RTT) {
		result->rtt = it2.get_u64();
	    } else if (type == GSCAN_ATTRIBUTE_RTTSD) {
		result->rtt_sd = it2.get_u64();
	    }
	}
    }

    if (i >= num) {
	ALOGE("Got too many results; skipping some");
    }
    return i;
}

int createFeatureRequest(WifiRequest& request, int subcmd, int enable) {

    int result = request.create(GOOGLE_OUI, subcmd);
    if (result < 0) {
	return result;
    }

    nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
    result = request.put_u32(GSCAN_ATTRIBUTE_ENABLE_FEATURE, enable);
    if (result < 0) {
	return result;
    }

    request.attr_end(data);
    return WIFI_SUCCESS;
}

class ScanCommand : public WifiCommand
{
    wifi_scan_cmd_params *mParams;
    wifi_scan_result_handler mHandler;
public:
    ScanCommand(wifi_interface_handle iface, int id, wifi_scan_cmd_params *params,
		wifi_scan_result_handler handler)
	: WifiCommand(iface, id), mParams(params), mHandler(handler)
	{ }

    int createSetupRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_CONFIG);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u32(GSCAN_ATTRIBUTE_BASE_PERIOD, mParams->base_period);
	if (result < 0) {
	    return result;
	}

	result = request.put_u32(GSCAN_ATTRIBUTE_NUM_BUCKETS, mParams->num_buckets);
	if (result < 0) {
	    return result;
	}

	for (int i = 0; i < mParams->num_buckets; i++) {
	    nlattr * bucket = request.attr_start(i);    // next bucket
	    result = request.put_u32(GSCAN_ATTRIBUTE_BUCKET_ID, mParams->buckets[i].bucket);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_BUCKET_PERIOD, mParams->buckets[i].period);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_BUCKETS_BAND,
		    mParams->buckets[i].band);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_REPORT_EVENTS,
		    mParams->buckets[i].report_events);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_BUCKET_NUM_CHANNELS,
		    mParams->buckets[i].num_channels);
	    if (result < 0) {
		return result;
	    }
	    if (mParams->buckets[i].num_channels) {
		nlattr *channels = request.attr_start(GSCAN_ATTRIBUTE_BUCKET_CHANNELS);
		for (int j = 0; j < mParams->buckets[i].num_channels; j++) {
		    result = request.put_u32(j, mParams->buckets[i].channels[j].channel);
		    if (result < 0) {
			return result;
		    }
		}
		request.attr_end(channels);
	    }

	    request.attr_end(bucket);
	}

	request.attr_end(data);
	return WIFI_SUCCESS;
    }

    int createScanConfigRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_SCAN_CONFIG);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u32(GSCAN_ATTRIBUTE_NUM_AP_PER_SCAN, mParams->max_ap_per_scan);
	if (result < 0) {
	    return result;
	}

	result = request.put_u32(GSCAN_ATTRIBUTE_REPORT_THRESHOLD,
			mParams->report_threshold_percent);
	if (result < 0) {
	    return result;
	}

	int num_scans = 20;
	for (int i = 0; i < mParams->num_buckets; i++) {
	    if (mParams->buckets[i].report_events == 1) {
		ALOGD("Setting num_scans to 1");
		num_scans = 1;
		break;
	    }
	}

	result = request.put_u32(GSCAN_ATTRIBUTE_NUM_SCANS_TO_CACHE, num_scans);
	if (result < 0) {
	    return result;
	}

	request.attr_end(data);
	return WIFI_SUCCESS;
    }

    int createStartRequest(WifiRequest& request) {
	return createFeatureRequest(request, GSCAN_SUBCMD_ENABLE_GSCAN, 1);
    }

    int createStopRequest(WifiRequest& request) {
	return createFeatureRequest(request, GSCAN_SUBCMD_ENABLE_GSCAN, 0);
    }

    int start() {
	ALOGD("1) GScan Setting configuration: ");
	WifiRequest request(familyId(), ifaceId());
	int result = createSetupRequest(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create setup request; result = %d", result);
	    return result;
	}

	result = requestResponse(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to configure setup; result = %d", result);
	    return result;
	}

	request.destroy();

	result = createScanConfigRequest(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create scan config request; result = %d", result);
	    return result;
	}

	result = requestResponse(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to configure scan; result = %d", result);
	    return result;
	}

	ALOGD("2) Enable GScan: ");

	result = createStartRequest(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create start request; result = %d", result);
	    return result;
	}

	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SCAN_RESULTS_AVAILABLE);
	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_COMPLETE_SCAN);
	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_FULL_SCAN_RESULTS);

	result = requestResponse(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to start scan; result = %d", result);
	    registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_COMPLETE_SCAN);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SCAN_RESULTS_AVAILABLE);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_FULL_SCAN_RESULTS);
	    return result;
	}
	return result;
    }

    virtual int cancel() {
	ALOGD("Stopping scan");

	WifiRequest request(familyId(), ifaceId());
	int result = createStopRequest(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create stop request; result = %d", result);
	} else {
	    result = requestResponse(request);
	    if (result != WIFI_SUCCESS) {
		ALOGE("failed to stop scan; result = %d", result);
	    }
	}

	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_COMPLETE_SCAN);
	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SCAN_RESULTS_AVAILABLE);
	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_FULL_SCAN_RESULTS);
	return WIFI_SUCCESS;
    }

    virtual int handleResponse(WifiEvent& reply) {
	/* Nothing to do on response! */
	return NL_SKIP;
    }

    virtual int handleEvent(WifiEvent& event) {
	ALOGD("[WIFI HAL]Got a scan results event");

	nlattr *vendor_data = (struct nlattr *)event.get_vendor_data();
	int len = event.get_vendor_data_len();
	int event_id = event.get_vendor_subcmd();

	ALOGV("vendor_data->nla_type=%d nla_len=%d, len=%d, event_id=%d",
	    vendor_data->nla_type, vendor_data->nla_len, len, event_id);

	if ((event_id == GSCAN_EVENT_COMPLETE_SCAN) ||
	    (event_id == GSCAN_EVENT_SCAN_RESULTS_AVAILABLE)) {
	    if (vendor_data == NULL || len != 4) {
		ALOGI("Bad event data!");
		return NL_SKIP;
	    }
	    wifi_scan_event evt_type;
	    if(vendor_data->nla_type == GSCAN_EVENT_COMPLETE_SCAN)
		evt_type = (wifi_scan_event) nla_get_u32(vendor_data);
	    ALOGV("Received event type %d", evt_type);
	    if(*mHandler.on_scan_event)
		(*mHandler.on_scan_event)(id(), evt_type);
	} else {
// TODO: This part is not used now. Vendor data type seems wrong.
	    if (vendor_data == NULL || len < sizeof(wifi_scan_result)) {
		ALOGE("No scan results found");
		return NL_SKIP;
	    }

	    ALOGD("vendor_data->nla_type=%d nla_len=%d, len=%d",
		   vendor_data->nla_type, vendor_data->nla_len, len);

	    wifi_gscan_full_result_t *drv_res = NULL;
	    if (vendor_data->nla_type == GSCAN_EVENT_FULL_SCAN_RESULTS)
		drv_res = (wifi_gscan_full_result_t *)nla_data(vendor_data);
	    /* To protect against corrupted data, put a ceiling */
	    int ie_len = min(MAX_PROBE_RESP_IE_LEN, drv_res->ie_length);
	    wifi_scan_result *full_scan_result;
	    wifi_gscan_result_t *fixed = &drv_res->fixed;

	    if ((ie_len + offsetof(wifi_gscan_full_result_t, ie_data)) > len) {
		ALOGE("BAD event data, len %d ie_len %d fixed length %d!\n", len,
		ie_len, offsetof(wifi_gscan_full_result_t, ie_data));
		return NL_SKIP;
	    }
	    full_scan_result = (wifi_scan_result *) malloc((ie_len + offsetof(wifi_scan_result, ie_data)));
	    if (!full_scan_result) {
		ALOGE("Full scan results: Can't malloc!\n");
		return NL_SKIP;
	    }
	    convert_to_hal_result(full_scan_result, fixed);
	    full_scan_result->ie_length = ie_len;
	    memcpy(full_scan_result->ie_data, drv_res->ie_data, ie_len);
	    if(mHandler.on_full_scan_result)
		mHandler.on_full_scan_result(id(), full_scan_result, drv_res->scan_ch_bucket);

	    ALOGV("Full scan result: %-32s %02x:%02x:%02x:%02x:%02x:%02x %d %d %lld %lld %lld %x %d\n",
		    fixed->ssid, fixed->bssid[0], fixed->bssid[1], fixed->bssid[2], fixed->bssid[3],
		    fixed->bssid[4], fixed->bssid[5], fixed->rssi, fixed->channel, fixed->ts,
		    fixed->rtt, fixed->rtt_sd, drv_res->scan_ch_bucket, drv_res->ie_length);
	    free(full_scan_result);
	}
	return NL_SKIP;
    }
};

wifi_error wifi_start_gscan(
	wifi_request_id id,
	wifi_interface_handle iface,
	wifi_scan_cmd_params params,
	wifi_scan_result_handler handler)
{
    wifi_handle handle = getWifiHandle(iface);

    ALOGD("[WIFI HAL]Starting GScan, halHandle = %p", handle);

    ScanCommand *cmd = new ScanCommand(iface, id, &params, handler);
    NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
    wifi_error result = wifi_register_cmd(handle, id, cmd);
    if (result != WIFI_SUCCESS) {
	return result;
    }
    return (wifi_error)cmd->start();
}

wifi_error wifi_stop_gscan(wifi_request_id id, wifi_interface_handle iface)
{
    ALOGD("[WIFI HAL]Stopping GScan");
    wifi_handle handle = getWifiHandle(iface);

    if (id == -1) {
	wifi_scan_result_handler handler;
	wifi_scan_cmd_params dummy_params;
	memset(&handler, 0, sizeof(handler));

	ScanCommand *cmd = new ScanCommand(iface, id, &dummy_params, handler);
	cmd->cancel();
	cmd->releaseRef();
	return WIFI_SUCCESS;
    }

    WifiCommand *cmd = wifi_unregister_cmd(handle, id);
    if (cmd) {
	cmd->cancel();
	cmd->releaseRef();
	return WIFI_SUCCESS;
    }

    return WIFI_ERROR_NOT_SUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////
class GetScanResultsCommand : public WifiCommand {
    wifi_scan_result *mResults;
    int mMax;
    int *mNum;
    int mRetrieved;
    byte mFlush;
    int mCompleted;
public:
    GetScanResultsCommand(wifi_interface_handle iface, byte flush,
	    wifi_scan_result *results, int max, int *num)
	: WifiCommand(iface, -1), mResults(results), mMax(max), mNum(num),
		mRetrieved(0), mFlush(flush), mCompleted(0)
    { }

    int createRequest(WifiRequest& request, int num, byte flush) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_GET_SCAN_RESULTS);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u32(GSCAN_ATTRIBUTE_NUM_OF_RESULTS, num);
	if (result < 0) {
	    return result;
	}

	result = request.put_u8(GSCAN_ATTRIBUTE_FLUSH_RESULTS, flush);
	if (result < 0) {
	    return result;
	}

	request.attr_end(data);
	return WIFI_SUCCESS;
    }

    int execute() {
	WifiRequest request(familyId(), ifaceId());
	ALOGD("retrieving %d scan results", mMax);

	for (int i = 0; i < 10 && mRetrieved < mMax; i++) {
	    int result = createRequest(request, (mMax - mRetrieved), mFlush);
	    if (result < 0) {
		ALOGE("failed to create request");
		return result;
	    }

	    int prev_retrieved = mRetrieved;

	    result = requestResponse(request);

	    if (result != WIFI_SUCCESS) {
		ALOGE("failed to retrieve scan results; result = %d", result);
		return result;
	    }

	    if (mRetrieved == prev_retrieved || mCompleted) {
		/* no more items left to retrieve */
		break;
	    }

	    request.destroy();
	}

	ALOGD("GetScanResults read %d results", mRetrieved);
	*mNum = mRetrieved;
	return WIFI_SUCCESS;
    }

    virtual int handleResponse(WifiEvent& reply) {
	ALOGD("In GetScanResultsCommand::handleResponse");

	if (reply.get_cmd() != NL80211_CMD_VENDOR) {
	    ALOGE("Ignoring reply with cmd = %d", reply.get_cmd());
	    return NL_SKIP;
	}

	int id = reply.get_vendor_id();
	int subcmd = reply.get_vendor_subcmd();

	/*
	if (subcmd != GSCAN_SUBCMD_SCAN_RESULTS) {
	    ALOGE("Invalid response to GetScanResultsCommand; ignoring it");
	    return NL_SKIP;
	}
	*/

	nlattr *vendor = reply.get_attribute(NL80211_ATTR_VENDOR_DATA);
	int len = reply.get_vendor_data_len();
	ALOGD("Id = %0x, subcmd = %d, vendor=%p, get_vendor_data()=%p vendor->nla_type=%d len=%d", 
		id, subcmd, vendor, reply.get_vendor_data(), vendor->nla_type, len);

	if (vendor == NULL || len == 0) {
	    ALOGE("no vendor data in GetScanResults response; ignoring it");
	    return NL_SKIP;
	}

	for (nl_iterator it(vendor); it.has_next(); it.next()) {
	    if (it.get_type() == GSCAN_ATTRIBUTE_SCAN_RESULTS_COMPLETE) {
		mCompleted = it.get_u8();
		ALOGI("retrieved mCompleted flag : %d", mCompleted);
	    } else if (it.get_type() == GSCAN_ATTRIBUTE_SCAN_RESULTS || it.get_type() == 0) {
		int scan_id = 0, flags = 0, num = 0, scan_ch_bucket_mask = 0;
		for (nl_iterator it2(it.get()); it2.has_next(); it2.next()) {
		    if (it2.get_type() == GSCAN_ATTRIBUTE_SCAN_ID) {
			scan_id = it.get_u32();
		    } else if (it2.get_type() == GSCAN_ATTRIBUTE_SCAN_FLAGS) {
			flags = it.get_u8();
		    } else if (it2.get_type() == GSCAN_ATTRIBUTE_NUM_OF_RESULTS) {
			num = it2.get_u32();
		    } else if (it2.get_type() == GSCAN_ATTRIBUTE_CH_BUCKET_BITMASK) {
			scan_ch_bucket_mask = it2.get_u32();
		    } else if (it2.get_type() == GSCAN_ATTRIBUTE_SCAN_RESULTS && num) {
			num = it2.get_len() / sizeof(wifi_gscan_result);
			num = min(*mNum - mRetrieved, num);
			ALOGD("Retrieved %d scan results", num);
			wifi_gscan_result_t *results = (wifi_gscan_result_t *)it2.get_data();
			wifi_scan_result *mScanResults = mResults + mRetrieved;
			for (int i = 0; i < num; i++) {
			    wifi_gscan_result_t *result = &results[i];
			    convert_to_hal_result(&mScanResults[i], result);
			    mScanResults[i].ie_length = 0;
			    ALOGD("%02d  %-32s  %02x:%02x:%02x:%02x:%02x:%02x  %04d channel=%d", i,
				result->ssid, result->bssid[0], result->bssid[1], result->bssid[2],
				result->bssid[3], result->bssid[4], result->bssid[5],
				result->rssi, result->channel);
			}
			mRetrieved += num;
		    } else {
			ALOGW("Ignoring invalid attribute type = %d, size = %d",
				it.get_type(), it.get_len());
		    }
		}
	    } else {
		ALOGW("Ignoring invalid attribute type = %d, size = %d",
			it.get_type(), it.get_len());
	    }
	}
	return NL_OK;
    }
};

wifi_error wifi_get_cached_gscan_results(wifi_interface_handle iface, byte flush,
	int max, wifi_scan_result *results, int *num) {
    ALOGD("[WIFI HAL]Getting cached scan results, iface handle = %p, num = %d", iface, *num);

    GetScanResultsCommand *cmd = new GetScanResultsCommand(iface, flush, results, max, num);
    NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
    return (wifi_error)cmd->execute();
}

wifi_error wifi_get_cached_gscan_results(wifi_interface_handle iface,
	byte flush, int max_scans,
	wifi_cached_scan_results *scans,
	int *num_scans)
{
    int num_scan_results = 1024;

    if (max_scans < 1 || scans == NULL || num_scans == NULL) {
	ALOGE("%s: no space to return results", __func__);
	return WIFI_ERROR_INVALID_ARGS;
    }

    wifi_scan_result *scan_results = static_cast<wifi_scan_result *>(
	malloc(num_scan_results * sizeof (wifi_scan_result)));
    if (scan_results == NULL) {
	ALOGE("%s: failed to allocate memory", __func__);
	return WIFI_ERROR_OUT_OF_MEMORY;
    }

    wifi_error result = wifi_get_cached_gscan_results(
	    iface, flush, num_scan_results, scan_results, &num_scan_results);
    if (result != WIFI_SUCCESS) {
	*num_scans = 0;
	free(scan_results);
	return result;
    }

    if (num_scan_results == 0) {
	*num_scans = 0;
	free(scan_results);
	return WIFI_SUCCESS;
    }

    /* TODO: This code tries to figure out number of scans based on timestamp */
    /* While this works in lot of cases, it doesn't always work well. It is best */
    /* to get the scan information from the firmware to be more accurate */

    const wifi_timestamp TDIFFMAX = 1600 * 1000;	    // 1.6 second
    *num_scans = 0;
    int j = 0;
    for (int i = 0; i < max_scans && j < num_scan_results; i++) {
	(*num_scans)++;
	scans[i].scan_id = 0;
	/* TODO: This should be set to 1 for truncated scans */
	scans[i].flags = 0;
	scans[i].num_results = 0;
	// scans[i].results = scan_results + j;

	wifi_timestamp ts = scan_results[j].ts;
	for ( ; j < num_scan_results; j++) {
	    wifi_timestamp tdiff = abs(scan_results[j].ts - ts);
	    if (tdiff < TDIFFMAX) {
		scans[i].num_results++;
	    }
	}
    }

    free(scan_results);

    if (j < num_scan_results) {
	ALOGW("%s: could not return all the results", __func__);
    }
    return WIFI_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
class BssidHotlistCommand : public WifiCommand
{
private:
    wifi_bssid_hotlist_params mParams;
    wifi_hotlist_ap_found_handler mHandler;
    static const int MAX_RESULTS = 64;
    wifi_scan_result mResults[MAX_RESULTS];
public:
    BssidHotlistCommand(wifi_interface_handle handle, int id,
	    wifi_bssid_hotlist_params params, wifi_hotlist_ap_found_handler handler)
	: WifiCommand(handle, id), mParams(params), mHandler(handler)
    { }

    int createSetupRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_HOTLIST);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u8(GSCAN_ATTRIBUTE_HOTLIST_FLUSH, 1);
	if (result < 0) {
	    return result;
	}

	result = request.put_u32(GSCAN_ATTRIBUTE_LOST_AP_SAMPLE_SIZE, mParams.lost_ap_sample_size);
	if (result < 0) {
	    return result;
	}

	result = request.put_u16(GSCAN_ATTRIBUTE_NUM_AP, mParams.num_bssid);
	if (result < 0) {
	    return result;
	}
	struct nlattr * attr = request.attr_start(GSCAN_ATTRIBUTE_HOTLIST_BSSIDS);
	for (int i = 0; i < mParams.num_bssid; i++) {
	    nlattr *attr2 = request.attr_start(GSCAN_ATTRIBUTE_HOTLIST_ELEM);
	    if (attr2 == NULL) {
		return WIFI_ERROR_OUT_OF_MEMORY;
	    }
	    result = request.put_addr(GSCAN_ATTRIBUTE_BSSID, mParams.ap[i].bssid);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_RSSI_HIGH, mParams.ap[i].high);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_RSSI_LOW, mParams.ap[i].low);
	    if (result < 0) {
		return result;
	    }
	    request.attr_end(attr2);
	}

	request.attr_end(attr);
	request.attr_end(data);
	return result;
    }

    int createTeardownRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_HOTLIST);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u8(GSCAN_ATTRIBUTE_HOTLIST_FLUSH, 1);
	if (result < 0) {
	    return result;
	}

	struct nlattr * attr = request.attr_start(GSCAN_ATTRIBUTE_HOTLIST_BSSIDS);
	request.attr_end(attr);
	request.attr_end(data);
	return result;
    }

    int start() {
	ALOGD("[WIFI HAL]Executing hotlist setup request, num = %d", mParams.num_bssid);
	WifiRequest request(familyId(), ifaceId());
	int result = createSetupRequest(request);
	if (result < 0) {
	    return result;
	}

	result = requestResponse(request);
	if (result < 0) {
	    ALOGI("Failed to execute hotlist setup request, result = %d", result);
	    //unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_FOUND);	// MTK??
	    //unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_LOST);
	    return result;
	}

	ALOGI("Successfully set %d APs in the hotlist", mParams.num_bssid);
	result = createFeatureRequest(request, GSCAN_SUBCMD_ENABLE_GSCAN, 1);
	if (result < 0) {
	    return result;
	}

	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_FOUND);
	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_LOST);

	result = requestResponse(request);
	if (result < 0) {
	    ALOGE("failed to start scan; result = %d", result);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_FOUND);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_LOST);
	    return result;
	}

	ALOGI("successfully restarted the scan");
	return result;
    }

    virtual int cancel() {
	/* unregister event handler */
	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_FOUND);
	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_HOTLIST_RESULTS_LOST);
	/* create set hotlist message with empty hotlist */
	WifiRequest request(familyId(), ifaceId());
	int result = createTeardownRequest(request);
	if (result < 0) {
	    return result;
	}

	result = requestResponse(request);
	if (result < 0) {
	    return result;
	}

	ALOGI("Successfully reset APs in current hotlist");
	return result;
    }

    virtual int handleResponse(WifiEvent& reply) {
	/* Nothing to do on response! */
	return NL_SKIP;
    }

    virtual int handleEvent(WifiEvent& event) {
	ALOGD("[WIFI HAL]Hotlist AP event");
	int event_id = event.get_vendor_subcmd();
	// event.log();

	struct nlattr *vendor_data = (struct nlattr *)event.get_vendor_data();
	int len = event.get_vendor_data_len();

	if (vendor_data == NULL || len == 0) {
	    ALOGI("No scan results found");
	    return NL_SKIP;
	}

	memset(mResults, 0, sizeof(wifi_scan_result) * MAX_RESULTS);

	int num = len / sizeof(wifi_gscan_result_t);
	wifi_gscan_result_t *inp = (wifi_gscan_result_t *)event.get_vendor_data();
	num = min(MAX_RESULTS, num);
	ALOGD("hotlist APs num=%d, vendor len=%d, sizeof()=%d, nla_len=%d nla_type=%d", 
	    num, len, sizeof(wifi_scan_result), vendor_data->nla_len, vendor_data->nla_type);

	if(vendor_data->nla_type == GSCAN_EVENT_HOTLIST_RESULTS_LOST ||
	   vendor_data->nla_type == GSCAN_EVENT_HOTLIST_RESULTS_FOUND)
	    for (int i = 0; i < num; i++, inp++) {
		convert_to_hal_result(&(mResults[i]), inp);
	    }

	if (event_id == GSCAN_EVENT_HOTLIST_RESULTS_FOUND) {
	    ALOGI("FOUND %d hotlist APs", num);
	    if (*mHandler.on_hotlist_ap_found)
		(*mHandler.on_hotlist_ap_found)(id(), num, mResults);
	} else if (event_id == GSCAN_EVENT_HOTLIST_RESULTS_LOST) {
	    ALOGI("LOST %d hotlist APs", num);
	    if (*mHandler.on_hotlist_ap_lost)
		(*mHandler.on_hotlist_ap_lost)(id(), num, mResults);
	}
	return NL_SKIP;
    }
};

wifi_error wifi_set_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface,
	wifi_bssid_hotlist_params params, wifi_hotlist_ap_found_handler handler)
{
    ALOGV("wifi_set_bssid_hotlist, wifi_request_id = %d", id);
    wifi_handle handle = getWifiHandle(iface);

    BssidHotlistCommand *cmd = new BssidHotlistCommand(iface, id, params, handler);
    NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
    wifi_error result = wifi_register_cmd(handle, id, cmd);
    if (result != WIFI_SUCCESS) {
	return result;
    }
    return (wifi_error)cmd->start();
}

wifi_error wifi_reset_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface)
{
    wifi_handle handle = getWifiHandle(iface);

    WifiCommand *cmd = wifi_unregister_cmd(handle, id);
    if (cmd) {
	cmd->cancel();
	cmd->releaseRef();
	return WIFI_SUCCESS;
    }

    return WIFI_ERROR_NOT_SUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////
class SignificantWifiChangeCommand : public WifiCommand
{
    typedef struct {
	mac_addr bssid;			// BSSID
	wifi_channel channel;		// channel frequency in MHz
	int num_rssi;			// number of rssi samples
	wifi_rssi rssi[8];		// RSSI history in db
    } wifi_significant_change_result_internal;

private:
    wifi_significant_change_params mParams;
    wifi_significant_change_handler mHandler;
    static const int MAX_RESULTS = 64;
    wifi_significant_change_result_internal mResultsBuffer[MAX_RESULTS];
    wifi_significant_change_result *mResults[MAX_RESULTS];
public:
    SignificantWifiChangeCommand(wifi_interface_handle handle, int id,
	    wifi_significant_change_params params, wifi_significant_change_handler handler)
	: WifiCommand(handle, id), mParams(params), mHandler(handler)
    { }

    int createSetupRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_SIGNIFICANT_CHANGE_CONFIG);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u8(GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_FLUSH, 1);
	if (result < 0) {
	    return result;
	}
	result = request.put_u16(GSCAN_ATTRIBUTE_RSSI_SAMPLE_SIZE, mParams.rssi_sample_size);
	if (result < 0) {
	    return result;
	}
	result = request.put_u16(GSCAN_ATTRIBUTE_LOST_AP_SAMPLE_SIZE, mParams.lost_ap_sample_size);
	if (result < 0) {
	    return result;
	}
	result = request.put_u16(GSCAN_ATTRIBUTE_MIN_BREACHING, mParams.min_breaching);
	if (result < 0) {
	    return result;
	}
	result = request.put_u16(GSCAN_ATTRIBUTE_NUM_AP, mParams.num_bssid);
	if (result < 0) {
	    return result;
	}

	struct nlattr * attr = request.attr_start(GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_BSSIDS);

	for (int i = 0; i < mParams.num_bssid; i++) {
	    nlattr *attr2 = request.attr_start(i);
	    if (attr2 == NULL) {
		return WIFI_ERROR_OUT_OF_MEMORY;
	    }
	    result = request.put_addr(GSCAN_ATTRIBUTE_BSSID, mParams.ap[i].bssid);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_RSSI_HIGH, mParams.ap[i].high);
	    if (result < 0) {
		return result;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_RSSI_LOW, mParams.ap[i].low);
	    if (result < 0) {
		return result;
	    }
	    request.attr_end(attr2);
	}

	request.attr_end(attr);
	request.attr_end(data);

	return result;
    }

    int createTeardownRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_SET_SIGNIFICANT_CHANGE_CONFIG);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u16(GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_FLUSH, 1);
	if (result < 0) {
	    return result;
	}

	request.attr_end(data);
	return result;
    }

    int start() {
	ALOGD("[WIFI HAL]Set significant wifi change config");
	WifiRequest request(familyId(), ifaceId());

	int result = createSetupRequest(request);
	if (result < 0) {
	    return result;
	}

	result = requestResponse(request);
	if (result < 0) {
	    ALOGE("failed to set significant wifi change config %d", result);
	    return result;
	}

	ALOGI("successfully set significant wifi change config");

	result = createFeatureRequest(request, GSCAN_SUBCMD_ENABLE_GSCAN, 1);
	if (result < 0) {
	    return result;
	}

	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SIGNIFICANT_CHANGE_RESULTS);

	result = requestResponse(request);
	if (result < 0) {
	    ALOGE("failed to start scan; result = %d", result);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SIGNIFICANT_CHANGE_RESULTS);
	    return result;
	}

	ALOGI("successfully restarted the scan");
	return result;
    }

    virtual int cancel() {
	/* unregister event handler */
	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_SIGNIFICANT_CHANGE_RESULTS);

	/* create set significant change monitor message with empty hotlist */
	WifiRequest request(familyId(), ifaceId());

	int result = createTeardownRequest(request);
	if (result < 0) {
	    return result;
	}

	result = requestResponse(request);
	if (result < 0) {
	    return result;
	}

	ALOGI("successfully reset significant wifi change config");
	return result;
    }

    virtual int handleResponse(WifiEvent& reply) {
	/* Nothing to do on response! */
	return NL_SKIP;
    }

    virtual int handleEvent(WifiEvent& event) {
	ALOGD("[WIFI HAL]Got a significant wifi change event");

	struct nlattr *vendor_data = (struct nlattr *)event.get_vendor_data();
	int len = event.get_vendor_data_len();

	if (vendor_data == NULL || len == 0) {
	    ALOGI("No scan results found");
	    return NL_SKIP;
	}

	typedef struct {
	    uint16_t flags;
	    uint16_t channel;
	    mac_addr bssid;
	    s8 rssi_history[8];
	} ChangeInfo;

	int num = min(len / sizeof(ChangeInfo), MAX_RESULTS);
	ChangeInfo *ci;
	if(vendor_data->nla_type == GSCAN_EVENT_SIGNIFICANT_CHANGE_RESULTS)
	    ci = (ChangeInfo *)nla_data(vendor_data);
	else
	    return NL_SKIP;

	for (int i = 0; i < num; i++) {
	    memcpy(mResultsBuffer[i].bssid, ci[i].bssid, sizeof(mac_addr));
	    mResultsBuffer[i].channel = ci[i].channel;
	    mResultsBuffer[i].num_rssi = 8;
	    for (int j = 0; j < mResultsBuffer[i].num_rssi; j++)
		mResultsBuffer[i].rssi[j] = (int) ci[i].rssi_history[j];
	    mResults[i] = reinterpret_cast<wifi_significant_change_result *>(&(mResultsBuffer[i]));
	}

	ALOGI("Retrieved %d scan results, vendor len=%d nla_type=%d", num, len, vendor_data->nla_type);

	if (num != 0) {
	    (*mHandler.on_significant_change)(id(), num, mResults);
	} else {
	    ALOGW("No significant change reported");
	}

	return NL_SKIP;
    }
};

wifi_error wifi_set_significant_change_handler(wifi_request_id id, wifi_interface_handle iface,
	wifi_significant_change_params params, wifi_significant_change_handler handler)
{
    wifi_handle handle = getWifiHandle(iface);

    SignificantWifiChangeCommand *cmd = new SignificantWifiChangeCommand(
	    iface, id, params, handler);
    NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
    wifi_error result = wifi_register_cmd(handle, id, cmd);
    if (result != WIFI_SUCCESS) {
	return result;
    }
    return (wifi_error)cmd->start();
}

wifi_error wifi_reset_significant_change_handler(wifi_request_id id, wifi_interface_handle iface)
{
    return wifi_cancel_cmd(id, iface);
}

wifi_error wifi_reset_epno_list(wifi_request_id id, wifi_interface_handle iface)
{
    return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_epno_list(wifi_request_id id, wifi_interface_handle iface,
	const wifi_epno_params *params, wifi_epno_handler handler)
{
    return WIFI_ERROR_NOT_SUPPORTED;
}
#if 0
TODO: not support in kernel driver yet
class BssidBlacklistCommand : public WifiCommand
{
private:
    wifi_bssid_params *mParams;
public:
    BssidBlacklistCommand(wifi_interface_handle handle, int id,
	    wifi_bssid_params *params)
	: WifiCommand(handle, id), mParams(params)
    { }
    int createRequest(WifiRequest& request) {
	int result = request.create(GOOGLE_OUI, WIFI_SUBCMD_SET_BSSID_BLACKLIST);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);
	result = request.put_u32(GSCAN_ATTRIBUTE_NUM_BSSID, mParams->num_bssid);
	if (result < 0) {
	    return result;
	}
	if (!mParams->num_bssid) {
	    result = request.put_u32(GSCAN_ATTRIBUTE_BSSID_BLACKLIST_FLUSH, 1);
	    if (result < 0) {
		return result;
	    }
	}
	for (int i = 0; i < mParams->num_bssid; i++) {
	    result = request.put_addr(GSCAN_ATTRIBUTE_BLACKLIST_BSSID, mParams->bssids[i]);
	    if (result < 0) {
		return result;
	    }
	}
	request.attr_end(data);
	return result;
    }

    int start() {
	ALOGV("Executing bssid blacklist request, num = %d", mParams->num_bssid);
	WifiRequest request(familyId(), ifaceId());
	int result = createRequest(request);
	if (result < 0) {
	    return result;
	}

	result = requestResponse(request);
	if (result < 0) {
	    ALOGE("Failed to execute bssid blacklist request, result = %d", result);
	    return result;
	}

	ALOGI("Successfully added %d blacklist bssids", mParams->num_bssid);
	if (result < 0) {
	    return result;
	}
	return result;
    }


    virtual int handleResponse(WifiEvent& reply) {
	/* Nothing to do on response! */
	return NL_SKIP;
    }
};
#endif

////////////////////////////////////////////////////////////////////////////////
class AnqpoConfigureCommand : public WifiCommand
{
    int num_hs;
    wifi_passpoint_network *mNetworks;
    wifi_passpoint_event_handler mHandler;
    wifi_scan_result *mResult;
public:
    AnqpoConfigureCommand(wifi_request_id id, wifi_interface_handle iface,
	int num, wifi_passpoint_network *hs_list, wifi_passpoint_event_handler handler)
	: WifiCommand(iface, id), num_hs(num), mNetworks(hs_list),
	    mHandler(handler)
    {
	mResult = NULL;
    }

    int createRequest(WifiRequest& request, int val) {
	int result = request.create(GOOGLE_OUI, GSCAN_SUBCMD_ANQPO_CONFIG);
	result = request.put_u32(GSCAN_ATTRIBUTE_ANQPO_HS_LIST_SIZE, num_hs);
	if (result < 0) {
	    return result;
	}

	nlattr *data = request.attr_start(NL80211_ATTR_VENDOR_DATA);

	struct nlattr * attr = request.attr_start(GSCAN_ATTRIBUTE_ANQPO_HS_LIST);
	for (int i = 0; i < num_hs; i++) {
	    nlattr *attr2 = request.attr_start(i);
	    if (attr2 == NULL) {
		return WIFI_ERROR_OUT_OF_MEMORY;
	    }
	    result = request.put_u32(GSCAN_ATTRIBUTE_ANQPO_HS_NETWORK_ID, mNetworks[i].id);
	    if (result < 0) {
		return result;
	    }
	    result = request.put(GSCAN_ATTRIBUTE_ANQPO_HS_NAI_REALM, mNetworks[i].realm, 256);
	    if (result < 0) {
		return result;
	    }
	    result = request.put(GSCAN_ATTRIBUTE_ANQPO_HS_ROAM_CONSORTIUM_ID,
			 mNetworks[i].roamingConsortiumIds, 128);
	    if (result < 0) {
		return result;
	    }
	    result = request.put(GSCAN_ATTRIBUTE_ANQPO_HS_PLMN, mNetworks[i].plmn, 3);
	    if (result < 0) {
		return result;
	    }

	    request.attr_end(attr2);
	}

	request.attr_end(attr);
	request.attr_end(data);

	return WIFI_SUCCESS;
    }

    int start() {
	WifiRequest request(familyId(), ifaceId());
	int result = createRequest(request, num_hs);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create request; result = %d", result);
	    return result;
	}

	registerVendorHandler(GOOGLE_OUI, GSCAN_EVENT_ANQPO_HOTSPOT_MATCH);

	result = requestResponse(request);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to set ANQPO networks; result = %d", result);
	    unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_ANQPO_HOTSPOT_MATCH);
	    return result;
	}

	return result;
    }

    virtual int cancel() {
	WifiRequest request(familyId(), ifaceId());
	int result = createRequest(request, 0);
	if (result != WIFI_SUCCESS) {
	    ALOGE("failed to create request; result = %d", result);
	} else {
	    result = requestResponse(request);
	    if (result != WIFI_SUCCESS) {
		ALOGE("failed to reset ANQPO networks;result = %d", result);
	    }
	}

	unregisterVendorHandler(GOOGLE_OUI, GSCAN_EVENT_ANQPO_HOTSPOT_MATCH);
	return WIFI_SUCCESS;
    }

    virtual int handleResponse(WifiEvent& reply) {
	 ALOGD("Request complete!");
	/* Nothing to do on response! */
	return NL_SKIP;
    }

    virtual int handleEvent(WifiEvent& event) {
	typedef struct {
	    u16 channel;	/* channel of GAS protocol */
	    u8  dialog_token;   /* GAS dialog token */
	    u8  fragment_id;    /* fragment id */
	    u16 status_code;    /* status code on GAS completion */
	    u16 data_len;       /* length of data to follow */
	    u8  data[1];	/* variable length specified by data_len */
	} wifi_anqp_gas_resp;

	ALOGI("ANQPO hotspot matched event!");

	nlattr *vendor_data = event.get_attribute(NL80211_ATTR_VENDOR_DATA);
	unsigned int len = event.get_vendor_data_len();

	if (vendor_data == NULL || len < sizeof(wifi_scan_result)) {
	    ALOGI("No scan results found");
	    return NL_SKIP;
	}
	mResult = (wifi_scan_result *)malloc(sizeof(wifi_scan_result));
	if (!mResult) {
	    return NL_SKIP;
	}
	wifi_gscan_full_result_t *drv_res = (wifi_gscan_full_result_t *)event.get_vendor_data();
	wifi_gscan_result_t *fixed = &drv_res->fixed;
	convert_to_hal_result(mResult, fixed);

	byte *anqp = (byte *)drv_res + offsetof(wifi_gscan_full_result_t, ie_data) + drv_res->ie_length;
	wifi_anqp_gas_resp *gas = (wifi_anqp_gas_resp *)anqp;
	int anqp_len = offsetof(wifi_anqp_gas_resp, data) + gas->data_len;
	int networkId = *(int *)((byte *)anqp + anqp_len);

	ALOGI("%-32s\t", mResult->ssid);

	ALOGI("%02x:%02x:%02x:%02x:%02x:%02x ", mResult->bssid[0], mResult->bssid[1],
		mResult->bssid[2], mResult->bssid[3], mResult->bssid[4], mResult->bssid[5]);

	ALOGI("%d\t", mResult->rssi);
	ALOGI("%d\t", mResult->channel);
	ALOGI("%lld\t", mResult->ts);
	ALOGI("%lld\t", mResult->rtt);
	ALOGI("%lld\n", mResult->rtt_sd);

	if(*mHandler.on_passpoint_network_found)
	    (*mHandler.on_passpoint_network_found)(id(), networkId, mResult, anqp_len, anqp);
	free(mResult);
	return NL_SKIP;
    }
};

wifi_error wifi_set_passpoint_list(wifi_request_id id, wifi_interface_handle iface, int num,
	wifi_passpoint_network *networks, wifi_passpoint_event_handler handler)
{
    wifi_handle handle = getWifiHandle(iface);

    AnqpoConfigureCommand *cmd = new AnqpoConfigureCommand(id, iface, num, networks, handler);
    NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
    wifi_error result = wifi_register_cmd(handle, id, cmd);
    if (result != WIFI_SUCCESS) {
	return result;
    }
    return (wifi_error)cmd->start();
}

wifi_error wifi_reset_passpoint_list(wifi_request_id id, wifi_interface_handle iface)
{
    return wifi_cancel_cmd(id, iface);
}
