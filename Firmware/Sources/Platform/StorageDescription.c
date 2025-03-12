// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] =
{
	{ "Reg 192. Device state", DT_Int16U, 1 },
	{ "Reg 193. Fault reason in the case DeviceState -> FAULT", DT_Int16U, 1 },
	{ "Reg 194. Disbale reason in the case DeviceState -> DISABLE", DT_Int16U, 1 },
	{ "Reg 195. Warning if present", DT_Int16U, 1 },
	{ "Reg 196. Problem if present", DT_Int16U, 1 },
	{ "Reg 198. Test result", DT_Int16U, 1 },
	{ "Reg 199. External fault code", DT_Int16U, 1 },
	{ "Reg 200. Charged summary", DT_Int16U, 1 },
	{ "Reg 201. Charged flag 1", DT_Int16U, 1 },
	{ "Reg 202. Charged flag 2", DT_Int16U, 1 },
	{ "Reg 203. Charged flag 3", DT_Int16U, 1 },
	{ "Reg 204. Charged flag 4", DT_Int16U, 1 },
	{ "Reg 205. Charged flag 5", DT_Int16U, 1 },
	{ "Reg 206. Charged flag 6", DT_Int16U, 1 },
	{ "Reg 207. Actual capacitor level 1", DT_Int16U, 1 },
	{ "Reg 208. Actual capacitor level 2", DT_Int16U, 1 },
	{ "Reg 209. Actual capacitor level 3", DT_Int16U, 1 },
	{ "Reg 210. Actual capacitor level 4", DT_Int16U, 1 },
	{ "Reg 211. Actual capacitor level 5", DT_Int16U, 1 },
	{ "Reg 212. Actual capacitor level 6", DT_Int16U, 1 },
	{ "Reg 213. Cell state 1", DT_Int16U, 1 },
	{ "Reg 214. Cell state 2", DT_Int16U, 1 },
	{ "Reg 215. Cell state 3", DT_Int16U, 1 },
	{ "Reg 216. Cell state 4", DT_Int16U, 1 },
	{ "Reg 217. Cell state 5", DT_Int16U, 1 },
	{ "Reg 218. Cell state 6", DT_Int16U, 1 }
};

Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);
