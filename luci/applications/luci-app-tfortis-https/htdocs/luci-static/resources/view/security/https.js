'use strict';
'require view';
'require form';

var enableLog = false;
function log(content){
    if (enableLog){
        return console.log("luci-app-nginx.servers: " + content);
    };
};
function isSSLEnabledInListenList(listenList){
    for (var i in listenList){
        if (listenList[i].includes("ssl")){
            return true;
        }
    }
    return false;
};

return view.extend({
    render: function (load_result){
        var cert_file = null;
        var key_file = null;
        var o;

        var formMap = new form.Map("https");

        var configSection = formMap.section(
            form.TypedSection, "server",
            _("HTTPS"),
            _("WEB Server Configuration")
        );
        configSection.addremove = false;
        configSection.anonymous = true;

        o = configSection.option(form.Flag, 'http_enable', _('Enable HTTP'));
        o = configSection.option(form.Value,'http_port', _('HTTP Port'));


        o = configSection.option(form.Flag, 'https_enable', _('Enable HTTPS'));
        o = configSection.option(form.Value,'https_port', _('HTTPS Port'));
        o = configSection.option(form.Flag, 'https_redirect', _('Redirect to HTTPS'),
            _('Forced redirect from the insecure version (HTTP) to the secure version (HTTPS) of WEB-interface'));
        o.depends({http_enable:"1",https_enable:"1"});

        o = configSection.option(form.Flag, 'local_only', _('Local access only'),
            _('Allow access only from local addresses'));

        cert_file = configSection.option(form.FileUpload, "ssl_certificate", _("SSL Certificate:"));
        cert_file.root_directory = '/etc/nginx/conf.d/';
        cert_file.enable_remove = false;

        key_file = configSection.option(form.FileUpload, 'ssl_certificate_key', _('SSL Certificate Key:'));
        key_file.root_directory = '/etc/nginx/conf.d/';
        key_file.enable_remove = false;

        var sslSessionCacheOption = configSection.option(
            form.Value, "ssl_session_cache",
            _("SSL Session Cache Option:")
        );
        sslSessionCacheOption.optional = true;
        var sslSessionTimeoutOption = configSection.option(
            form.Value, "ssl_session_timeout",
            _("SSL Session Timeout:")
        );
        sslSessionTimeoutOption.optional = true;

        return formMap.render();
    }
});
