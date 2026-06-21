#!/usr/bin/python3
import os, sys, gi
gi.require_version("Gio","2.0")
from gi.repository import Gio, GLib
PCT="/run/mt6582-bl-pct"; SI="org.gnome.SettingsDaemon.Power.Screen"
def log(m):
    try: open("/tmp/shimdbg.log","a").write(m+"\n")
    except Exception: pass
def getpct():
    try: return max(0,min(100,int(open(PCT).read().strip())))
    except Exception: return 75
def setpct(v):
    try:
        with open(PCT,"w") as f: f.write(str(max(0,min(100,int(v)))))
        log("setpct OK -> %d"%int(v))
    except Exception as e: log("setpct FAIL %r"%e)
XML='<node><interface name="'+SI+'"><property name="Brightness" type="i" access="readwrite"/><method name="StepUp"><arg type="i" direction="out"/></method><method name="StepDown"><arg type="i" direction="out"/></method></interface></node>'
def on_method(conn,sender,path,iface,method,params,inv):
    log("METHOD %s"%method)
    if method=="StepUp": setpct(min(100,getpct()+5)); inv.return_value(GLib.Variant("(i)",(getpct(),)))
    elif method=="StepDown": setpct(max(0,getpct()-5)); inv.return_value(GLib.Variant("(i)",(getpct(),)))
    else: inv.return_value(None)
def on_get(conn,sender,path,iface,prop):
    log("GET %s"%prop); return GLib.Variant("i",getpct()) if prop=="Brightness" else None
def on_set(conn,sender,path,iface,prop,value):
    log("ON_SET %s = %r"%(prop,value))
    if prop=="Brightness": setpct(value.get_int32()); return True
    return False
addr=os.environ.get("DBUS_SESSION_BUS_ADDRESS")
log("=== start pid=%d uid=%d addr=%s"%(os.getpid(),os.getuid(),addr))
conn=Gio.DBusConnection.new_for_address_sync(addr, Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT|Gio.DBusConnectionFlags.MESSAGE_BUS_CONNECTION, None, None)
loop=GLib.MainLoop(); conn.connect("closed", lambda *a: loop.quit())
node=Gio.DBusNodeInfo.new_for_xml(XML)
conn.register_object("/org/gnome/SettingsDaemon/Power", node.interfaces[0], on_method, on_get, on_set)
def acq(c,n): log("ACQUIRED %s uniq=%s"%(n,conn.get_unique_name()))
def lost(c,n): log("LOST %s"%n)
Gio.bus_own_name_on_connection(conn, "org.gnome.SettingsDaemon.Power", Gio.BusNameOwnerFlags.ALLOW_REPLACEMENT|Gio.BusNameOwnerFlags.REPLACE, acq, lost)
loop.run()
