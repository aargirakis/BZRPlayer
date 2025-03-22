--- src/core/plugins/players/plugins_list.h	2018-12-18 14:10:47.000000000 +0100
+++ src/core/plugins/players/plugins_list.h	2025-03-19 13:51:11.977986100 +0100
@@ -44,23 +44,6 @@
   void RegisterFTCSupport(PlayerPluginsRegistrator& registrator);
   void RegisterCOPSupport(PlayerPluginsRegistrator& registrator);
   void RegisterTFESupport(PlayerPluginsRegistrator& registrator);
-  void RegisterXMPPlugins(PlayerPluginsRegistrator& registrator);
-  void RegisterSIDPlugins(PlayerPluginsRegistrator& registrator);
-  void RegisterET1Support(PlayerPluginsRegistrator& registrator);
-  void RegisterAYCSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterSPCSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterMTCSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterGMEPlugins(PlayerPluginsRegistrator& registrator);
-  void RegisterAHXSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterPSFSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterUSFSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterGSFSupport(PlayerPluginsRegistrator& registrator);
-  void Register2SFSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterSDSFSupport(PlayerPluginsRegistrator& registrator);
-  void RegisterASAPPlugins(PlayerPluginsRegistrator& registrator);
-  void RegisterMP3Plugin(PlayerPluginsRegistrator& registrator);
-  void RegisterOGGPlugin(PlayerPluginsRegistrator& registrator);
-  void RegisterWAVPlugin(PlayerPluginsRegistrator& registrator);
 
   void RegisterPlayerPlugins(PlayerPluginsRegistrator& registrator)
   {
@@ -94,22 +77,5 @@
     RegisterFTCSupport(registrator);
     RegisterCOPSupport(registrator);
     RegisterTFESupport(registrator);
-    RegisterXMPPlugins(registrator);
-    RegisterSIDPlugins(registrator);
-    RegisterET1Support(registrator);
-    RegisterAYCSupport(registrator);
-    RegisterSPCSupport(registrator);
-    RegisterMTCSupport(registrator);
-    RegisterGMEPlugins(registrator);
-    RegisterAHXSupport(registrator);
-    RegisterPSFSupport(registrator);
-    RegisterUSFSupport(registrator);
-    RegisterGSFSupport(registrator);
-    Register2SFSupport(registrator);
-    RegisterSDSFSupport(registrator);
-    RegisterASAPPlugins(registrator);
-    RegisterMP3Plugin(registrator);
-    RegisterOGGPlugin(registrator);
-    RegisterWAVPlugin(registrator);
   }
 }
