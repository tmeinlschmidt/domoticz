#pragma once

#include "DomoticzHardware.h"

class MySensorsBase : public CDomoticzHardwareBase
{
	friend class MySensorsSerial;
	friend class MySensorsTCP;
	friend class MQTT;
public:
	enum _eMessageType
	{
		MT_Presentation = 0,	// Sent by a node when they present attached sensors.This is usually done in setup() at startup.
		MT_Set = 1,			// This message is sent from or to a sensor when a sensor value should be updated
		MT_Req = 2,			// Requests a variable value(usually from an actuator destined for controller).
		MT_Internal = 3,		// This is a special internal message.See table "_eInternalType" below for the details
		MT_Stream = 4			// Used for OTA firmware updates
	};

	enum _ePresentationType
	{
		S_DOOR = 0,						//Door and window sensors	V_TRIPPED, V_ARMED
		S_MOTION = 1,					//Motion sensors	V_TRIPPED, V_ARMED
		S_SMOKE = 2,					//Smoke sensor	V_TRIPPED, V_ARMED
		S_LIGHT = 3,					//Light Actuator(on / off)	V_STATUS(or V_LIGHT), V_WATT
		S_BINARY = 3,					//Binary device(on / off), Alias for S_LIGHT	V_STATUS(or V_LIGHT), V_WATT
		S_DIMMER = 4,					//Dimmable device of some kind	V_STATUS(on / off), V_DIMMER(dimmer level 0 - 100), V_WATT
		S_COVER = 5,					//Window covers or shades	V_UP, V_DOWN, V_STOP, V_PERCENTAGE
		S_TEMP = 6,						//Temperature sensor	V_TEMP, V_ID
		S_HUM = 7,						//Humidity sensor	V_HUM
		S_BARO = 8,						//Barometer sensor(Pressure)	V_PRESSURE, V_FORECAST
		S_WIND = 9,						//Wind sensor	V_WIND, V_GUST
		S_RAIN = 10,					//Rain sensor	V_RAIN, V_RAINRATE
		S_UV = 11,						//UV sensor	V_UV
		S_WEIGHT = 12,					//Weight sensor for scales etc.V_WEIGHT, V_IMPEDANCE
		S_POWER = 13,					//Power measuring device, like power meters	V_WATT, V_KWH
		S_HEATER = 14,					//Heater device	V_HVAC_SETPOINT_HEAT, V_HVAC_FLOW_STATE, V_TEMP, V_STATUS
		S_DISTANCE = 15,				//Distance sensor	V_DISTANCE, V_UNIT_PREFIX
		S_LIGHT_LEVEL = 16,				//Light sensor	V_LIGHT_LEVEL(uncalibrated percentage), V_LEVEL(light level in lux)
		S_ARDUINO_NODE = 17,			//Arduino node device
		S_ARDUINO_REPEATER_NODE = 18,	//Arduino repeating node device
		S_LOCK = 19,					//Lock device	V_LOCK_STATUS
		S_IR = 20,						//Ir sender / receiver device	V_IR_SEND, V_IR_RECEIVE
		S_WATER = 21,					//Water meter	V_FLOW, V_VOLUME
		S_AIR_QUALITY = 22,				//Air quality sensor e.g.MQ - 2	V_LEVEL, V_UNIT_PREFIX
		S_CUSTOM = 23,					//Use this for custom sensors where no other fits.
		S_DUST = 24,					//Dust level sensor	V_LEVEL, V_UNIT_PREFIX
		S_SCENE_CONTROLLER = 25,		//Scene controller device	V_SCENE_ON, V_SCENE_OFF
		S_RGB_LIGHT = 26,				//RGB light	V_RGB, V_WATT
		S_RGBW_LIGHT = 27,				//RGBW light(with separate white component)	V_RGBW, V_WATT
		S_COLOR_SENSOR = 28,			//Color sensor	V_RGB
		S_HVAC = 29,					//Thermostat / HVAC device	V_HVAC_SETPOINT_HEAT, V_HVAC_SETPOINT_COLD, V_HVAC_FLOW_STATE, V_HVAC_FLOW_MODE, V_HVAC_SPEED, V_TEMP, V_STATUS
		S_MULTIMETER = 30,				//Multimeter device	V_VOLTAGE, V_CURRENT, V_IMPEDANCE
		S_SPRINKLER = 31,				//Sprinkler device	V_STATUS(turn on / off), V_TRIPPED(if fire detecting device)
		S_WATER_LEAK = 32,				//Water leak sensor	V_TRIPPED, V_ARMED
		S_SOUND = 33,					//Sound sensor	V_LEVEL(in dB), V_TRIPPED, V_ARMED
		S_VIBRATION = 34,				//Vibration sensor	V_LEVEL(vibration in Hz), V_TRIPPED, V_ARMED
		S_MOISTURE = 35,				//Moisture sensor	V_LEVEL(water content or moisture in percentage ? ), V_TRIPPED, V_ARMED

		S_UNKNOWN = 200,				//No Type received
	};

	enum _eSetType
	{
		V_TEMP = 0,						//Temperature	S_TEMP, S_HEATER, S_HVAC
		V_HUM = 1,						//Humidity	S_HUM
		V_STATUS = 2,					//Binary status. 0 = off 1 = on	S_LIGHT, S_DIMMER, S_SPRINKLER, S_HVAC, S_HEATER
		V_LIGHT = 2,					//Deprecated.Alias for V_STATUS.Light status. 0 = off 1 = on	S_LIGHT, S_DIMMER, S_SPRINKLER
		V_PERCENTAGE = 3,				//Percentage value. 0 - 100 (%)	S_DIMMER
		V_DIMMER = 3,					//Deprecated.Alias for V_PERCENTAGE.Dimmer value. 0 - 100 (%)	S_DIMMER
		V_PRESSURE = 4,					//Atmospheric Pressure	S_BARO
		V_FORECAST = 5,					//Whether forecast.One of "stable", "sunny", "cloudy", "unstable", "thunderstorm" or "unknown"	S_BARO
		V_RAIN = 6,						//Amount of rain	S_RAIN
		V_RAINRATE = 7,					//Rate of rain	S_RAIN
		V_WIND = 8,						//Wind speed	S_WIND
		V_GUST = 9,						//Gust	S_WIND
		V_DIRECTION = 10,				//Wind direction	S_WIND
		V_UV = 11,						//UV light level	S_UV
		V_WEIGHT = 12,					//Weight(for scales etc)	S_WEIGHT
		V_DISTANCE = 13,				//Distance	S_DISTANCE
		V_IMPEDANCE = 14,				//Impedance value	S_MULTIMETER, S_WEIGHT
		V_ARMED = 15,					//Armed status of a security sensor. 1 = Armed, 0 = Bypassed	S_DOOR, S_MOTION, S_SMOKE, S_SPRINKLER, S_WATER_LEAK, S_SOUND, S_VIBRATION, S_MOISTURE
		V_TRIPPED = 16,					//Tripped status of a security sensor. 1 = Tripped, 0 = Untripped	S_DOOR, S_MOTION, S_SMOKE, S_SPRINKLER, S_WATER_LEAK, S_SOUND, S_VIBRATION, S_MOISTURE
		V_WATT = 17,					//Watt value for power meters	S_POWER, S_LIGHT, S_DIMMER, S_RGB, S_RGBW
		V_KWH = 18,						//Accumulated number of KWH for a power meter	S_POWER
		V_SCENE_ON = 19,				//Turn on a scene	S_SCENE_CONTROLLER
		V_SCENE_OFF = 20,				//Turn of a scene	S_SCENE_CONTROLLER
		V_HVAC_FLOW_STATE = 21,			//Mode of header.One of "Off", "HeatOn", "CoolOn", or "AutoChangeOver"	S_HVAC, S_HEATER
		V_HVAC_SPEED = 22,				//HVAC / Heater fan speed("Min", "Normal", "Max", "Auto")	S_HVAC, S_HEATER
		V_LIGHT_LEVEL = 23,				//Uncalibrated light level. 0 - 100 % .Use V_LEVEL for light level in lux.S_LIGHT_LEVEL
		V_VAR1 = 24,					//Custom value	Any device
		V_VAR2 = 25,					//Custom value	Any device
		V_VAR3 = 26,					//Custom value	Any device
		V_VAR4 = 27,					//Custom value	Any device
		V_VAR5 = 28,					//Custom value	Any device
		V_UP = 29,						//Window covering.Up.S_COVER
		V_DOWN = 30,					//Window covering.Down.S_COVER
		V_STOP = 31,					//Window covering.Stop.S_COVER
		V_IR_SEND = 32,					//Send out an IR - command	S_IR
		V_IR_RECEIVE = 33,				//This message contains a received IR - command	S_IR
		V_FLOW = 34,					//Flow of water(in meter)	S_WATER
		V_VOLUME = 35,					//Water volume	S_WATER
		V_LOCK_STATUS = 36,				//Set or get lock status. 1 = Locked, 0 = Unlocked	S_LOCK
		V_LEVEL = 37,					//Used for sending level - value	S_DUST, S_AIR_QUALITY, S_SOUND(dB), S_VIBRATION(hz), S_LIGHT_LEVEL(lux)
		V_VOLTAGE = 38,					//Voltage level	S_MULTIMETER
		V_CURRENT = 39,					//Current level	S_MULTIMETER
		V_RGB = 40,						//RGB value transmitted as ASCII hex string(I.e "ff0000" for red)	S_RGB_LIGHT, S_COLOR_SENSOR
		V_RGBW = 41,					//RGBW value transmitted as ASCII hex string(I.e "ff0000ff" for red + full white)	S_RGBW_LIGHT
		V_ID = 42,						//Optional unique sensor id(e.g.OneWire DS1820b ids)	S_TEMP
		V_UNIT_PREFIX = 43,				//Allows sensors to send in a string representing the unit prefix to be displayed in GUI.This is not parsed by controller!E.g.cm, m, km, inch.S_DISTANCE, S_DUST, S_AIR_QUALITY
		V_HVAC_SETPOINT_COOL = 44,		//HVAC cold setpoint(Integer between 0 - 100)	S_HVAC
		V_HVAC_SETPOINT_HEAT = 45,		//HVAC / Heater setpoint(Integer between 0 - 100)	S_HVAC, S_HEATER
		V_HVAC_FLOW_MODE = 46,			//Flow mode for HVAC("Auto", "ContinuousOn", "PeriodicOn")	S_HVAC

		V_UNKNOWN = 200					//No value received
	};

	enum _eInternalType
	{
		I_BATTERY_LEVEL = 0,			//Use this to report the battery level(in percent 0 - 100).
		I_TIME = 1,						//Sensors can request the current time from the Controller using this message.The time will be reported as the seconds since 1970
		I_VERSION = 2,					//Used to request gateway version from controller.
		I_ID_REQUEST = 3,				//Use this to request a unique node id from the controller.
		I_ID_RESPONSE = 4,				//Id response back to sensor.Payload contains sensor id.
		I_INCLUSION_MODE = 5,			//Start / stop inclusion mode of the Controller(1 = start, 0 = stop).
		I_CONFIG = 6,					//Config request from node.Reply with(M)etric or (I)mperal back to sensor.
		I_FIND_PARENT = 7,				//When a sensor starts up, it broadcast a search request to all neighbor nodes.They reply with a I_FIND_PARENT_RESPONSE.
		I_FIND_PARENT_RESPONSE = 8,		//Reply message type to I_FIND_PARENT request.
		I_LOG_MESSAGE = 9,				//Sent by the gateway to the Controller to trace - log a message
		I_CHILDREN = 10,				//A message that can be used to transfer child sensors(from EEPROM routing table) of a repeating node.
		I_SKETCH_NAME = 11,				//Optional sketch name that can be used to identify sensor in the Controller GUI
		I_SKETCH_VERSION = 12,			//Optional sketch version that can be reported to keep track of the version of sensor in the Controller GUI.
		I_REBOOT = 13,					//Used by OTA firmware updates.Request for node to reboot.
		I_GATEWAY_READY = 14,			//Send by gateway to controller when startup is complete.
		I_REQUEST_SIGNING = 15,			//Used between sensors when initiating signing.
		I_GET_NONCE = 16,				//Used between sensors when requesting nonce.
		I_GET_NONCE_RESPONSE = 17,		//Used between sensors for nonce response.
	};

	struct _tMySensorValue
	{
		float floatValue;
		int intvalue;
		bool bValidValue;
		std::string stringValue;

		_tMySensorValue()
		{
			floatValue = 0;
			intvalue = 0;
			bValidValue = false;
		}
	};

	struct _tMySensorChild
	{
		int nodeID;
		int childID;

		_ePresentationType presType;
		std::string childName;
		bool useAck;

		//values
		std::map<_eSetType, _tMySensorValue> values;
		//battery
		bool hasBattery;
		int batValue;

		time_t lastreceived;

		_tMySensorChild()
		{
			nodeID = -1;
			childID = 1;
			hasBattery = false;
			batValue = 255;
			presType = S_UNKNOWN;
			useAck = false;
		}

		bool GetValue(const _eSetType vType, int &intValue)
		{
			std::map<_eSetType, _tMySensorValue>::const_iterator itt;
			itt = values.find(vType);
			if (itt == values.end())
				return false;
			if (!itt->second.bValidValue)
				return false;
			intValue = itt->second.intvalue;
			return true;
		}
		bool GetValue(const _eSetType vType, float &floatValue)
		{
			std::map<_eSetType, _tMySensorValue>::const_iterator itt;
			itt = values.find(vType);
			if (itt == values.end())
				return false;
			if (!itt->second.bValidValue)
				return false;
			floatValue = itt->second.floatValue;
			return true;
		}
		bool GetValue(const _eSetType vType, std::string &stringValue)
		{
			std::map<_eSetType, _tMySensorValue>::const_iterator itt;
			itt = values.find(vType);
			if (itt == values.end())
				return false;
			if (!itt->second.bValidValue)
				return false;
			stringValue = itt->second.stringValue;
			return true;
		}
		void SetValue(const _eSetType vType, const int intValue)
		{
			values[vType].intvalue = intValue;
			values[vType].bValidValue = true;
		}
		void SetValue(const _eSetType vType, const float floatValue)
		{
			values[vType].floatValue = floatValue;
			values[vType].bValidValue = true;
		}
		void SetValue(const _eSetType vType, const std::string &stringValue)
		{
			values[vType].stringValue = stringValue;
			values[vType].bValidValue = true;
		}
	};

	struct _tMySensorNode
	{
		int nodeID;
		std::string SketchName;
		std::string SketchVersion;
		time_t lastreceived;
		std::vector<_tMySensorChild> m_childs;
		_tMySensorNode()
		{
			lastreceived = 0;
			nodeID = -1;
		}
		_tMySensorChild* FindChildWithPresentationType(const _ePresentationType cType)
		{
			std::vector<_tMySensorChild>::iterator itt;
			for (itt = m_childs.begin(); itt != m_childs.end(); ++itt)
			{
				if (itt->presType == cType)
				{
					return &*itt;
				}
			}
			return NULL;
		}
		_tMySensorChild* FindChildWithPresentationType(const int ChildID, const _ePresentationType cType)
		{
			std::vector<_tMySensorChild>::iterator itt;
			for (itt = m_childs.begin(); itt != m_childs.end(); ++itt)
			{
				if ((itt->childID == ChildID) &&
					(itt->presType == cType)
					)
				{
					return &*itt;
				}
			}
			return NULL;
		}
		_tMySensorChild* FindChildWithValueType(const int ChildID, const _eSetType valType)
		{
			std::vector<_tMySensorChild>::iterator itt;
			for (itt = m_childs.begin(); itt != m_childs.end(); ++itt)
			{
				if (itt->childID == ChildID)
				{
					std::map<_eSetType, _tMySensorValue>::const_iterator itt2;
					for (itt2 = itt->values.begin(); itt2 != itt->values.end(); ++itt2)
					{
						if (itt2->first == valType)
						{
							if (!itt2->second.bValidValue)
								return NULL;
							return &*itt;
						}
					}
				}
			}
			return NULL;
		}
		_tMySensorChild* FindChild(const int ChildID)
		{
			std::vector<_tMySensorChild>::iterator itt;
			for (itt = m_childs.begin(); itt != m_childs.end(); ++itt)
			{
				if (itt->childID == ChildID)
				{
					return &*itt;
				}
			}
			return NULL;
		}
	} MySensorNode;

	MySensorsBase(void);
	~MySensorsBase(void);
	std::string m_szSerialPort;
	unsigned int m_iBaudRate;
	bool WriteToHardware(const char *pdata, const unsigned char length);
private:
	virtual void WriteInt(const std::string &sendStr) = 0;
	void ParseData(const unsigned char *pData, int Len);
	void ParseLine();

	void UpdateChildDBInfo(const int NodeID, const int ChildID, const _ePresentationType pType, const std::string &Name, const bool UseAck);
	bool GetChildDBInfo(const int NodeID, const int ChildID, _ePresentationType &pType, std::string &Name, bool &UseAck);

	void SendCommand(const int NodeID, const int ChildID, const _eMessageType messageType, const int SubType, const std::string &Payload);
	void UpdateSwitch(const unsigned char Idx, const int SubUnit, const bool bOn, const double Level, const std::string &defaultname);

	bool GetSwitchValue(const unsigned char Idx, const int SubUnit, const int sub_type, std::string &sSwitchValue);

	bool GetBlindsValue(const int NodeID, const int ChildID, int &blind_value);

	void LoadDevicesFromDatabase();
	void Add2Database(const int nodeID, const std::string &SketchName, const std::string &SketchVersion);
	void DatabaseUpdateSketchName(const int nodeID, const std::string &SketchName);
	void DatabaseUpdateSketchVersion(const int nodeID, const std::string &SketchVersion);

	void SendSensor2Domoticz(_tMySensorNode *pNode, _tMySensorChild *pSensor, const _eSetType vType);

	void MakeAndSendWindSensor(const int nodeID, const std::string &sname);

	_tMySensorNode* FindNode(const int nodeID);
	_tMySensorNode* InsertNode(const int nodeID);
	void RemoveNode(const int nodeID);
	int FindNextNodeID();
	_tMySensorChild* FindSensorWithPresentationType(const int nodeID, const _ePresentationType presType);
	_tMySensorChild* FindChildWithValueType(const int nodeID, const _eSetType valType);
	void InsertSensor(_tMySensorChild device);
	void UpdateNodeBatteryLevel(const int nodeID, const int Level);

	void UpdateVar(const int NodeID, const int ChildID, const int VarID, const std::string &svalue);
	bool GetVar(const int NodeID, const int ChildID, const int VarID, std::string &sValue);

	std::map<int, _tMySensorNode> m_nodes;

	static const int readBufferSize=1028;
	unsigned char m_buffer[readBufferSize];
	int m_bufferpos;
};

