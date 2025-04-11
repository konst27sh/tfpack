'use strict';
'require form';
'require fs';
'require view';
'require rpc';
'require poll';
'require uci';


var callGetBuiltinEthernetPorts = rpc.declare({
    object: 'luci',
    method: 'getBuiltinEthernetPorts',
    expect: { result: [] }
});

function callBridgeStatus(port){
    return fs.exec_direct('/usr/sbin/bridge',['link','show','dev',port]).then(function(res){
        return res;
    });
}


var portlist;
var ubus_objects = [];

function initUbusStatus() {
    var objects = [];
    var sections = uci.sections('8021x', 'interface');

    for (var i = 0; i < sections.length; i++){
        if(sections[i]['enable'] == '1'){
            var ifname = sections[i]['ifname'];
            objects.push('hostapd.'+ifname);
        }
    }
    return (objects.length ? L.resolveDefault(rpc.list.apply(rpc, objects), {}) : Promise.resolve({})).then(function(res) {
        for (var k in res) {
            var m = k.match(/^hostapd\.(.+)$/);
            if(m)
                ubus_objects.push(m);
        }
        return ubus_objects;
    });
}


function renderStatus(ifname,data) {
    var view = document.getElementById("status_"+ifname);
    if(view != null){
        if(data){
            if(data['status'] == "ENABLED"){
                view.innerHTML = "<b>Running</b>";
            }
            else
                view.innerHTML = "-";
        }
    }
}
function renderPortStatus(ifname,data) {
    var view = document.getElementById("status_"+ifname);
    if(view != null){
        if(data.includes('forwarding')){
            view.innerHTML += "<br>Forwarding";
        }
        else{
            view.innerHTML += "<br>Blocked";
        }
    }
}


function renderClients(ifname, data) {
    var view = document.getElementById("status_"+ifname);
    if (typeof  data['clients'] !== 'undefined') {
        var names = Object.keys(data['clients']);
        if(names.length){
            view.innerHTML += "<br>Connected clients:<br>";
            for (var i = 0; i < names.length; i++){
                view.innerHTML += names[i] + ": ";
                if(data['clients'][names[i]]['authorized'])
                    view.innerHTML += "<span style=\"color:green\">authorized</span>";
                else
                    view.innerHTML += "<span style=\"color:red\">not authorized</span>";
            }
        }
    }
}


return L.view.extend({
    /** @private */
    renderPortList: function(data) {
        portlist = data;
    },

    getHostapdStatus: function(ifname) {
        var run = rpc.declare({
            object: 'hostapd.'+ifname,
            method: 'get_status',
            expect: {}
        });
        return run().then(function(data) {
            renderStatus(ifname,data);
        });
    },
    getHostapdPortStatus: function(ifname) {
        callBridgeStatus(ifname).then(function(data) {
            renderPortStatus(ifname,data);
        });
    },

    getHostapdClients: function(ifname) {
        var run = rpc.declare({
            object: 'hostapd.'+ifname,
            method: 'get_clients',
            expect: {}
        });
        return run().then(function(data) {
            renderClients(ifname,data);
        });
    },


    startPolling: function() {
        initUbusStatus();
        poll.add(L.bind(function() {
            for (var i = 0; i < ubus_objects.length; i++){
                this.getHostapdStatus(ubus_objects[i][1]);
                this.getHostapdPortStatus(ubus_objects[i][1]);
                this.getHostapdClients(ubus_objects[i][1]);
            }
        }, this),5);

        poll.add(L.bind(function() {

        }, this),5);
    },


    load: function() {
        return callGetBuiltinEthernetPorts().then(L.bind(function(data) {
            this.renderPortList(data);
        }, this));
    },


    render: function() {
        var  m, s, o;

        m = new form.Map('8021x', _("802.1x"),_('Configure IEEE 802.1x wired authentication'));
        m.tabbed = true;

        s = m.section(form.TypedSection, 'general', _('General'));
        s.anonymous = true;
        s.readonly = true;

        o = s.option( form.Flag, 'enable', _('Enable '));
        o.default = o.disabled;
        o.editable = true;

        o = s.option(form.Value, 'auth_server_addr', _('Authentication server'),_('RADIUS authentication server'));
        o.editable = true;

        o = s.option(form.Value, 'auth_server_port', _('Authentication port'));
        o.editable = true;

        o = s.option(form.Value, 'auth_server_shared_secret', _('Authentication shared secret'));
        o.editable = true;

        o = s.option(form.Value, 'acct_server_addr', _('Accounting server'),_('RADIUS accounting  server. Empty if not used'));
        o.editable = true;

        o = s.option(form.Value, 'acct_server_port', _('Accounting port'));
        o.editable = true;

        o = s.option(form.Value, 'acct_server_shared_secret', _('Accounting shared secret'));
        o.editable = true;

        o = s.option(form.Value, 'eap_reauth_period', _('EAP reauthentication period'));
        o.editable = true;

        o = s.option(form.FileUpload, 'ca_cert', _('CA certificate'),_('(PEM or DER file) for EAP-TLS/PEAP/TTLS'));
        o.root_directory = '/etc/radius';
        o.enable_remove = false;
        o.enable_upload = true;

        o = s.option(form.FileUpload, 'server_cert', _('Server certificate'),_('(PEM or DER file) for EAP-TLS/PEAP/TTLS'));
        o.root_directory = '/etc/radius';
        o.enable_remove = false;
        o.enable_upload = true;

        o = s.option(form.FileUpload, 'private_key', _('Private key'),_("Private key matching with the server certificate for EAP-TLS/PEAP/TTLS.	This may point to the same file as server_cert if both certificate and key are included in a single file"));
        o.root_directory = '/etc/radius';
        o.enable_remove = false;
        o.enable_upload = true;


        s = m.section(form.GridSection, 'interface', _('Interfaces'));
        s.anonymous = true;
        s.readonly = true;

        o = s.tab('settings', _('Settings'));

        o = s.taboption('settings',form.DummyValue, 'ifname', 'Interface');
        o.cfgvalue = function(section_id) {
            return getPortName(section_id);
        }
        o.width = '20%';

        o = s.taboption('settings',form.ListValue, 'enable', _('802.1X State'));
        o.value("0","Disable");
        o.value("1","Enable");
        o.editable = true;
        o.width = '20%';

        o = s.taboption('settings', form.DummyValue, 'status', _('Status'));
        o.editable = true;
        o.modalonly = false;
        o.render = L.bind(function(self, section, scope) {
            return 	E('td', { class: 'td cbi-value-field', id: 'status_'+section}, _('-'));
        }, this);


        s = m.section(form.TypedSection, 'logs', _('Logging'));
        s.anonymous = true;
        o = s.option(form.ListValue, 'logger_syslog_level', _('Logging level'));
        o.value("0","Verbose debugging");
        o.value("1","Debugging");
        o.value("2","Informational");
        o.value("3","Notification");
        o.value("4","Warning");
        o.editable = true;


        return m.render().then(L.bind(function(rendered) {
            this.startPolling();
            return rendered;
        }, this));
    }
});
