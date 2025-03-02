#include "JSONDescription.h"
#include "FormatOutputJSON.h"

const char TemplateJSON[] = "[{\n"
"	'param' : 'dVdt',\n"
"	'type' : 'set',\n"
"	'ranges': [\n"
"		{\n"
"			'rangeId' : '1',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '20'\n"
"		},\n"
"		{\n"
"			'rangeId' : '2',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '50',\n"
"		},\n"
"		{\n"
"			'rangeId' : '3',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '200',\n"
"		},\n"
"		{\n"
"			'rangeId' : '4',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '320',\n"
"		},\n"
"		{\n"
"			'rangeId' : '5',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '500',\n"
"		},\n"
"		{\n"
"			'rangeId' : '6',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '1000',\n"
"		},\n"
"		{\n"
"			'rangeId' : '7',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '1600',\n"
"		},\n"
"		{\n"
"			'rangeId' : '8',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '2000',\n"
"		},\n"
"		{\n"
"			'rangeId' : '9',\n"
"			'unitsMultiply' : '1',\n"
"			'active' : '$',\n"
"			'value': '2500',\n"
"		}]\n"
"}]";

Int16U Rate20_Active;
Int16U Rate50_Active;
Int16U Rate200_Active;
Int16U Rate320_Active;
Int16U Rate500_Active;
Int16U Rate1000_Active;
Int16U Rate1600_Active;
Int16U Rate2000_Active;
Int16U Rate2500_Active;

Int16U JSONPointers[JSON_POINTERS_SIZE] = {0};

