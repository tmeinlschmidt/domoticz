#include "stdafx.h"
#include "NetatmoWeatherStation.h"
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "hardwaretypes.h"
#include "../main/localtime_r.h"
#include "../httpclient/HTTPClient.h"
#include "../json/json.h"

#define round(a) ( int ) ( a + .5 )

#ifdef _DEBUG
	//#define DEBUG_NetatmoWeatherStation
#endif

#ifdef DEBUG_NetatmoWeatherStation
void SaveString2Disk(std::string str, std::string filename)
{
	FILE *fOut = fopen(filename.c_str(), "wb+");
	if (fOut)
	{
		fwrite(str.c_str(), 1, str.size(), fOut);
		fclose(fOut);
	}
}
std::string ReadFile(std::string filename)
{
	std::ifstream file;
	std::string sResult = "";
	file.open(filename.c_str());
	if (!file.is_open())
		return "";
	std::string sLine;
	while (!file.eof())
	{
		getline(file, sLine);
		sResult += sLine;
	}
	file.close();
	return sResult;
}
#endif

CNetAtmoWeatherStation::CNetAtmoWeatherStation(const int ID, const std::string& username, const std::string& password) :
m_username(username),
m_password(password)
{
	m_nextRefreshTs = mytime(NULL);
	m_isLogged = false;

	m_HwdID=ID;

	m_clientId = "5588029e485a88af28f4a3c4";
	m_clientSecret = "6vIpQVjNsL2A74Bd8tINscklLw2LKv7NhE9uW2";
	m_stoprequested=false;

	Init();
}

CNetAtmoWeatherStation::~CNetAtmoWeatherStation(void)
{
}

void CNetAtmoWeatherStation::Init()
{
	m_RainOffset=0;
	m_OldRainCounter=0;
}

bool CNetAtmoWeatherStation::StartHardware()
{
	Init();
	//Start worker thread
	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CNetAtmoWeatherStation::Do_Work, this)));
	m_bIsStarted=true;
	sOnConnected(this);
	return (m_thread!=NULL);
}

bool CNetAtmoWeatherStation::StopHardware()
{
	if (m_thread!=NULL)
	{
		m_stoprequested = true;
		m_thread->join();
	}
    m_bIsStarted=false;
    return true;
}

void CNetAtmoWeatherStation::Do_Work()
{
	int sec_counter = 28;
	bool bFirstTime = true;
	_log.Log(LOG_STATUS, "Netatmo: Worker started...");
	while (!m_stoprequested)
	{
		sleep_seconds(1);
		if (m_stoprequested)
			break;
		sec_counter++;
		if (sec_counter % 12 == 0) {
			m_LastHeartbeat = mytime(NULL);
		}

		if (!m_isLogged)
		{
			if (sec_counter % 30 == 0)
			{
				Login();
			}
		}
		if (m_isLogged)
		{
			if (RefreshToken())
			{
				if ((sec_counter % 240 == 0) || (bFirstTime))
				{
					bFirstTime = false;
					GetMeterDetails();
				}
			}
		}
	}
	_log.Log(LOG_STATUS,"Netatmo: Worker stopped...");
}

static unsigned int crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static unsigned int Crc32(unsigned int crc, const unsigned char *buf, size_t size)
{
	const unsigned char *p = buf;
	crc = crc ^ ~0U;
	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return crc ^ ~0U;
}

bool CNetAtmoWeatherStation::Login()
{
	if (m_isLogged)
		return true;

	std::stringstream sstr;
	sstr << "grant_type=password&";
	sstr << "client_id=" << m_clientId << "&";
	sstr << "client_secret=" << m_clientSecret << "&";
	sstr << "username=" << m_username << "&";
	sstr << "password=" << m_password << "&";
	sstr << "scope=read_station";

	std::string httpData = sstr.str();
	std::vector<std::string> ExtraHeaders;

	ExtraHeaders.push_back("Host: api.netatmo.net");
	ExtraHeaders.push_back("Content-Type: application/x-www-form-urlencoded;charset=UTF-8");

	std::string httpUrl("https://api.netatmo.net/oauth2/token");
	std::string sResult;
	bool ret = HTTPClient::POST(httpUrl, httpData, ExtraHeaders, sResult);
	if (!ret)
	{
		_log.Log(LOG_ERROR, "Netatmo: Error connecting to Server...");
		return false;
	}

	Json::Value root;
	Json::Reader jReader;
	ret = jReader.parse(sResult, root);
	if (!ret)
	{
		_log.Log(LOG_ERROR, "Netatmo: Invalid/no data received...");
		return false;
	}

	if (root["access_token"].empty() || root["expires_in"].empty() || root["refresh_token"].empty())
	{
		_log.Log(LOG_ERROR, "Netatmo: No access granted, check username/password...");
		return false;
	}

	m_accessToken = root["access_token"].asString();
	m_refreshToken = root["refresh_token"].asString();
	int expires = root["expires_in"].asInt();
	m_nextRefreshTs = mytime(NULL) + expires;
	m_isLogged = true;
	return true;
}

bool CNetAtmoWeatherStation::RefreshToken()
{
	if (!m_isLogged)
		return false;

	if ((mytime(NULL) - 15) < m_nextRefreshTs)
		return true; //no need to refresh the token yet

	// Time to refresh the token
	std::stringstream sstr;
	sstr << "grant_type=refresh_token&";
	sstr << "refresh_token=" << m_refreshToken << "&";
	sstr << "client_id=" << m_clientId << "&";
	sstr << "client_secret=" << m_clientSecret;

	std::string httpData = sstr.str();
	std::vector<std::string> ExtraHeaders;

	ExtraHeaders.push_back("Host: api.netatmo.net");
	ExtraHeaders.push_back("Content-Type: application/x-www-form-urlencoded;charset=UTF-8");

	std::string httpUrl("https://api.netatmo.net/oauth2/token");
	std::string sResult;
	bool ret = HTTPClient::POST(httpUrl, httpData, ExtraHeaders, sResult);
	if (!ret)
	{
		_log.Log(LOG_ERROR, "Netatmo: Error connecting to Server...");
		return false;
	}

	Json::Value root;
	Json::Reader jReader;
	ret = jReader.parse(sResult, root);
	if (!ret)
	{
		_log.Log(LOG_ERROR, "Netatmo: Invalid/no data received...");
		//Force login next time
		m_isLogged = false;
		return false;
	}

	if (root["access_token"].empty() || root["expires_in"].empty() || root["refresh_token"].empty())
	{
		//Force login next time
		_log.Log(LOG_ERROR, "Netatmo: No access granted, forcing login again...");
		m_isLogged = false;
		return false;
	}

	m_accessToken = root["access_token"].asString();
	m_refreshToken = root["refresh_token"].asString();
	int expires = root["expires_in"].asInt();
	m_nextRefreshTs = mytime(NULL) + expires;
	return true;
}

int CNetAtmoWeatherStation::GetBatteryLevel(const std::string &ModuleType, const int battery_vp)
{
	int batValue = 255;
	if (battery_vp == 0)
		return batValue; //no battery

	bool bIsIndoorSensor = ((ModuleType == "NAMain") || (ModuleType == "NAModule4"));
	bool bIsOutdoorSensor = ((ModuleType == "NAModule1") || (ModuleType == "NAModule3"));
	bool bIsWindGaugeSensor = (ModuleType == "NAModule2");
	bool bIsThermostatSensor = (ModuleType == "NATherm1");

	if (bIsIndoorSensor)
	{
		/* Battery range: 6000 ... 4200
		5640 full
		5280 high
		4920 medium
		4560 low
		Below 4560: very low */
		if (battery_vp <= 4560)
			batValue = 0;
	}
	else if (bIsOutdoorSensor)
	{
		/*Battery range : 6000 ... 3600 * /
		5500 full
		5000 high
		4500 medium
		4000 low
		below 4000: very low */
		if (battery_vp <= 4000)
			batValue = 0;
	}
	else if (bIsWindGaugeSensor)
	{
		/* Battery range: 6000 ... 3950
		5590 full
		5180 high
		4770 medium
		4360 low
		below 4360: very low*/
		if (battery_vp <= 4360)
			batValue = 0;
	}
	else if (bIsThermostatSensor)
	{
		/* Battery range: 4500 ... 3000
		4100 full
		3600 high
		3300 medium
		3000 low
		/* below 3000: very low */
		if (battery_vp <= 3000)
			batValue = 0;
	}
	return batValue;
}

bool CNetAtmoWeatherStation::ParseDashboard(const Json::Value &root, const int ID, const std::string &name, const std::string &ModuleType, const int battery_vp)
{
	bool bHaveTemp = false;
	bool bHaveHum = false;
	bool bHaveBaro = false;
	bool bHaveCO2 = false;
	bool bHaveRain = false;
	bool bHaveSound = false;
	bool bHaveWind = false;

	float temp;
	int hum;
	float baro;
	int co2;
	float rain;
	int sound;

	float wind_angle = 0;
	int wind_gust_angle = 0;
	float wind_strength = 0;
	float wind_gust = 0;

	int batValue = GetBatteryLevel(ModuleType, battery_vp);

	if (!root["Temperature"].empty())
	{
		bHaveTemp = true;
		temp = root["Temperature"].asFloat();
	}
	if (!root["Humidity"].empty())
	{
		bHaveHum = true;
		hum = root["Humidity"].asInt();
	}
	if (!root["Pressure"].empty())
	{
		bHaveBaro = true;
		baro = root["Pressure"].asFloat();
	}
	if (!root["Noise"].empty())
	{
		bHaveSound = true;
		sound = root["Noise"].asInt();
	}
	if (!root["CO2"].empty())
	{
		bHaveCO2 = true;
		co2 = root["CO2"].asInt();
	}
	if (!root["sum_rain_24"].empty())
	{
		bHaveRain = true;
		rain = root["sum_rain_24"].asFloat();
	}
	if (!root["WindAngle"].empty())
	{
		if (
			(!root["WindAngle"].empty()) &&
			(!root["WindStrength"].empty()) &&
			(!root["GustAngle"].empty()) &&
			(!root["GustStrength"].empty())
			)
		{
			bHaveWind = true;
			wind_angle = float(root["WindAngle"].asInt())/16.0f;
			wind_gust_angle = root["GustAngle"].asInt();
			wind_strength = root["WindStrength"].asFloat()/ 3.6f;
			wind_gust = root["GustStrength"].asFloat() / 3.6f;
		}
	}

	if (bHaveTemp && bHaveHum && bHaveBaro)
	{
		int nforecast = wsbaroforcast_some_clouds;
		float pressure = baro;
		if (pressure <= 980)
			nforecast = wsbaroforcast_heavy_rain;
		else if (pressure <= 995)
		{
			if (temp > 1)
				nforecast = wsbaroforcast_rain;
			else
				nforecast = wsbaroforcast_snow;
		}
		else if (pressure >= 1029)
			nforecast = wsbaroforcast_sunny;
		SendTempHumBaroSensorFloat(ID, batValue, temp, hum, baro, nforecast, name);
	}
	else if (bHaveTemp && bHaveHum)
	{
		SendTempHumSensor(ID, batValue, temp, hum, name);
	}
	else if (bHaveTemp)
	{
		SendTempSensor(ID, batValue, temp, name);
	}

	if (bHaveRain)
	{
		if ((m_RainOffset == 0) && (m_OldRainCounter == 0))
		{
			//get last rain counter from the database
			bool bExists=false;
			m_RainOffset = GetRainSensorValue(ID, bExists);
			m_RainOffset -= rain;
			if (m_RainOffset < 0)
				m_RainOffset = 0;
		}
		if (rain < m_OldRainCounter)
		{
			//daily counter went to zero
			m_RainOffset += m_OldRainCounter;
		}
		m_OldRainCounter = rain;
		SendRainSensor(ID, batValue, m_RainOffset+ m_OldRainCounter, name);
	}

	if (bHaveCO2)
	{
		SendAirQualitySensor((ID & 0xFF00) >> 8, ID & 0xFF, batValue, co2, name);
	}

	if (bHaveSound)
	{
		SendSoundSensor(ID, batValue, sound, name);
	}
	
	if (bHaveWind)
	{
		SendWind(ID, batValue, wind_angle, wind_strength, wind_gust, 0, 0, false, name);
	}
	return true;
}

void CNetAtmoWeatherStation::GetMeterDetails()
{
	if (!m_isLogged)
		return;

	std::stringstream sstr2;
	sstr2 << "https://api.netatmo.net/api/devicelist";
	sstr2 << "?";
	sstr2 << "access_token=" << m_accessToken;
	sstr2 << "&" << "get_favorites=" << "true";
	std::string httpUrl = sstr2.str();

	std::vector<std::string> ExtraHeaders;
	std::string sResult;

#ifdef DEBUG_NetatmoWeatherStation
	sResult = ReadFile("E:\\netatmo_mdetails.json");
	bool ret = true;
#else
	bool ret=HTTPClient::GET(httpUrl, ExtraHeaders, sResult);
	if (!ret)
	{
		_log.Log(LOG_STATUS, "Netatmo: Error connecting to Server...");
		return;
	}
#endif
#ifdef DEBUG_NetatmoWeatherStation2
	SaveString2Disk(sResult, "E:\\netatmo_mdetails.json");
#endif
	Json::Value root;
	Json::Reader jReader;
	ret=jReader.parse(sResult,root);
	if (!ret)
	{
		_log.Log(LOG_STATUS, "Netatmo: Invalid data received...");
		return;
	}

	if (!root["body"].empty() && !root["body"]["devices"].empty())
	{
		if (root["body"]["devices"].isArray())
		{
			for (Json::Value::iterator itDevice=root["body"]["devices"].begin(); itDevice!=root["body"]["devices"].end(); ++itDevice)
			{
				Json::Value device = *itDevice;
				if (!device["_id"].empty())
				{
					if (!device["dashboard_data"].empty())
					{
						std::string id = device["_id"].asString();
						std::string type = device["type"].asString();
						std::string name = device["module_name"].asString();
						stdreplace(name, "'", "");

						int battery_vp = 0;
						if (device["battery_vp"].empty() == false)
						{
							battery_vp = device["battery_vp"].asInt();
						}
						//std::set<std::string> dataTypes;
						//for (Json::Value::iterator itDataType = device["data_type"].begin(); itDataType != device["data_type"].end(); ++itDataType)
						//{
						//	dataTypes.insert((*itDataType).asCString());
						//}
						int crcId = Crc32(0, (const unsigned char *)id.c_str(), id.length());
						ParseDashboard(device["dashboard_data"], crcId, name, type, battery_vp);
						//getData(type, name, dataTypes, id, std::string(""), battery_vp);
					}
				}
			}
		}
	}
	if (!root["body"].empty() && !root["body"]["modules"].empty())
	{
		if (root["body"]["modules"].isArray())
		{
			for (Json::Value::iterator itModule=root["body"]["modules"].begin(); itModule!=root["body"]["modules"].end(); ++itModule)
			{
				Json::Value module = *itModule;
				if (!module["_id"].empty())
				{
					if (!module["dashboard_data"].empty())
					{
						std::string id = module["_id"].asString();
						std::string type = module["type"].asString();
						std::string deviceId = module["main_device"].asString();
						std::string name = module["module_name"].asString();
						int battery_vp = 0;
						if (module["battery_vp"] == false)
						{
							battery_vp = module["battery_vp"].asInt();
						}
						stdreplace(name, "'", " ");

						//std::set<std::string> dataTypes;
						//for (Json::Value::iterator itDataType = module["data_type"].begin(); itDataType != module["data_type"].end(); ++itDataType)
						//{
						//	dataTypes.insert((*itDataType).asCString());
						//}
						int crcId = Crc32(0, (const unsigned char *)id.c_str(), id.length());
						ParseDashboard(module["dashboard_data"], crcId, name, type, battery_vp);
						//getData(type, name, dataTypes, deviceId, id, battery_vp);
					}
				}
			}
		}
	}
}

