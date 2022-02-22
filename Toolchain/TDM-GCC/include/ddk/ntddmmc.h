
#pragma once

typedef enum _FEATURE_PROFILE_TYPE {
  ProfileInvalid = 0,
  ProfileNonRemovableDisk = 1,
  ProfileRemovableDisk = 2,
  ProfileMOErasable = 3,
  ProfileMOWriteOnce = 4,
  ProfileAS_MO = 5,
  ProfileCdrom = 8,
  ProfileCdRecordable = 9,
  ProfileCdRewritable = 0xa,
  ProfileDvdRom = 0x10,
  ProfileDvdRecordable = 0x11,
  ProfileDvdRam = 0x12,
  ProfileDvdRewritable = 0x13,
  ProfileDvdRWSequential = 0x14,
  ProfileDvdDashRDualLayer = 0x15,
  ProfileDvdDashRLayerJump = 0x16,
  ProfileDvdPlusRW = 0x1A,
  ProfileDvdPlusR = 0x1B,
  ProfileDDCdrom = 0x20,
  ProfileDDCdRecordable = 0x21,
  ProfileDDCdRewritable = 0x22,
  ProfileDvdPlusRWDualLayer = 0x2A,
  ProfileDvdPlusRDualLayer = 0x2B,
  ProfileBDRom = 0x40,
  ProfileBDRSequentialWritable = 0x41,
  ProfileBDRRandomWritable = 0x42,
  ProfileBDRewritable = 0x43,
  ProfileHDDVDRom = 0x50,
  ProfileHDDVDRecordable = 0x51,
  ProfileHDDVDRam = 0x52,
  ProfileHDDVDRewritable = 0x53,
  ProfileHDDVDRDualLayer = 0x58,
  ProfileHDDVDRWDualLayer = 0x5A,
  ProfileNonStandard = 0xffff
} FEATURE_PROFILE_TYPE, *PFEATURE_PROFILE_TYPE;

typedef enum _FEATURE_NUMBER {
  FeatureProfileList              = 0x0000,
  FeatureCore                     = 0x0001,
  FeatureMorphing                 = 0x0002,
  FeatureRemovableMedium          = 0x0003,
  FeatureWriteProtect             = 0x0004,
  FeatureRandomReadable           = 0x0010,
  FeatureMultiRead                = 0x001D,
  FeatureCdRead                   = 0x001E,
  FeatureDvdRead                  = 0x001F,
  FeatureRandomWritable           = 0x0020,
  FeatureIncrementalStreamingWritable = 0x0021,
  FeatureSectorErasable           = 0x0022,
  FeatureFormattable              = 0x0023,
  FeatureDefectManagement         = 0x0024,
  FeatureWriteOnce                = 0x0025,
  FeatureRestrictedOverwrite      = 0x0026,
  FeatureCdrwCAVWrite             = 0x0027,
  FeatureMrw                      = 0x0028,
  FeatureEnhancedDefectReporting  = 0x0029,
  FeatureDvdPlusRW                = 0x002A,
  FeatureDvdPlusR                 = 0x002B,
  FeatureRigidRestrictedOverwrite = 0x002C,
  FeatureCdTrackAtOnce            = 0x002D,
  FeatureCdMastering              = 0x002E,
  FeatureDvdRecordableWrite       = 0x002F,
  FeatureDDCDRead                 = 0x0030,
  FeatureDDCDRWrite               = 0x0031,
  FeatureDDCDRWWrite              = 0x0032,
  FeatureLayerJumpRecording       = 0x0033,
  FeatureCDRWMediaWriteSupport    = 0x0037,
  FeatureBDRPseudoOverwrite       = 0x0038,
  FeatureDvdPlusRWDualLayer       = 0x003A,
  FeatureDvdPlusRDualLayer        = 0x003B,
  FeatureBDRead                   = 0x0040,
  FeatureBDWrite                  = 0x0041,
  FeatureTSR                      = 0x0042,
  FeatureHDDVDRead                = 0x0050,
  FeatureHDDVDWrite               = 0x0051,
  FeatureHybridDisc               = 0x0080,
  FeaturePowerManagement          = 0x0100,
  FeatureSMART                    = 0x0101,
  FeatureEmbeddedChanger          = 0x0102,
  FeatureCDAudioAnalogPlay        = 0x0103,
  FeatureMicrocodeUpgrade         = 0x0104,
  FeatureTimeout                  = 0x0105,
  FeatureDvdCSS                   = 0x0106,
  FeatureRealTimeStreaming        = 0x0107,
  FeatureLogicalUnitSerialNumber  = 0x0108,
  FeatureMediaSerialNumber        = 0x0109,
  FeatureDiscControlBlocks        = 0x010A,
  FeatureDvdCPRM                  = 0x010B,
  FeatureFirmwareDate             = 0x010C,
  FeatureAACS                     = 0x010D,
  FeatureVCPS                     = 0x0110,
} FEATURE_NUMBER, *PFEATURE_NUMBER;

#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL     0x0
#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT 0x1
#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE     0x2

typedef struct _GET_CONFIGURATION_HEADER {
    UCHAR DataLength[4];
    UCHAR Reserved[2];
    UCHAR CurrentProfile[2];
#if !defined(__midl)
    UCHAR Data[0];
#endif
} GET_CONFIGURATION_HEADER, *PGET_CONFIGURATION_HEADER;

typedef struct _FEATURE_HEADER {
    UCHAR FeatureCode[2];
    UCHAR Current    : 1;
    UCHAR Persistent : 1;
    UCHAR Version    : 4;
    UCHAR Reserved0  : 2;
    UCHAR AdditionalLength;
} FEATURE_HEADER, *PFEATURE_HEADER;

typedef struct _FEATURE_DATA_PROFILE_LIST_EX {
    UCHAR ProfileNumber[2];
    UCHAR Current                   : 1;
    UCHAR Reserved1                 : 7;
    UCHAR Reserved2;
} FEATURE_DATA_PROFILE_LIST_EX, *PFEATURE_DATA_PROFILE_LIST_EX;

typedef struct _FEATURE_DATA_PROFILE_LIST {
    FEATURE_HEADER Header;
#if !defined(__midl)
    FEATURE_DATA_PROFILE_LIST_EX Profiles[0];
#endif
} FEATURE_DATA_PROFILE_LIST, *PFEATURE_DATA_PROFILE_LIST;
