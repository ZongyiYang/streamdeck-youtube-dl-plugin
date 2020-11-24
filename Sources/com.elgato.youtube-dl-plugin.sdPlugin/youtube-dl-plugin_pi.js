// this is our global websocket, used to communicate from/to Stream Deck software
// and some info about our plugin, as sent by Stream Deck software
var websocket = null,
uuid = null,
actionInfo = {};

function connectElgatoStreamDeckSocket(inPort, inUUID, inRegisterEvent, inInfo, inActionInfo) {
    uuid = inUUID;
    // please note: the incoming arguments are of type STRING, so
    // in case of the inActionInfo, we must parse it into JSON first
    actionInfo = JSON.parse(inActionInfo); // cache the info
    websocket = new WebSocket('ws://localhost:' + inPort);

    // if connection was established, the websocket sends
    // an 'onopen' event, where we need to register our PI
    websocket.onopen = function () {
        var json = {
            event:  inRegisterEvent,
            uuid:   inUUID
        };
        // register property inspector to Stream Deck
        websocket.send(JSON.stringify(json));

        json = {
            "event": "getSettings",
            "context": uuid,
        };
        websocket.send(JSON.stringify(json));
    };

    // retrieve saved settings if there are any
    websocket.onmessage = function (evt) {
        // Received message from Stream Deck
        const jsonObj = JSON.parse(evt.data);

        if (jsonObj.event === 'didReceiveSettings') {
            const payload = jsonObj.payload.settings;

            if (payload.label !== undefined)
                document.getElementById('label_textbox').value = payload.label;

            if (payload.type !== undefined)
                document.getElementById('download_type_menu').value = payload.type;
            else
                document.getElementById('download_type_menu').value = "Video";

            if (payload.maxDownloads !== undefined)
                document.getElementById('max_downloads_textbox').value = payload.maxDownloads;
            else
                document.getElementById('max_downloads_textbox').value = 1;

            if (payload.outputFolder !== undefined)
                document.getElementById('output_folder_textbox').value = payload.outputFolder;

            if (payload.youtubeDlExePath !== undefined)
                document.getElementById('youtubedl_path_textbox').value = payload.youtubeDlExePath;

            if (payload.customCommand !== undefined)
                document.getElementById('cmd_textbox').value = payload.customCommand;

            updateSettingsToPlugin();
        }

		if (jsonObj.event === 'sendToPropertyInspector') {
			const payload = jsonObj.payload;
			if (payload.sampleCommand !== undefined)
			{
				document.getElementById('sample_command_textbox').value = payload.sampleCommand;
			}
		}
    };
}

// send a command to plugin
function sendCommand(command)
{
	payload = {'command':command};
	sendValueToPlugin(payload);
}

// update settings
function updateSettingsToPlugin() {
    payload = {
            'label':document.getElementById('label_textbox').value,
            'type':document.getElementById('download_type_menu').value,
            'maxDownloads':document.getElementById('max_downloads_textbox').value,
            'customCommand':document.getElementById('cmd_textbox').value,
            'outputFolder':document.getElementById('output_folder_textbox').value,
            'youtubeDlExePath':document.getElementById('youtubedl_path_textbox').value,
	};

    sendValueToPlugin(payload);
    saveValues(payload);
	sendCommand('getSampleCommand'); // retrieve sample command
}

// send a payload to plugin
function sendValueToPlugin(payload) {
    if (websocket) {
        const json = {
                "action": actionInfo['action'],
                "event": "sendToPlugin",
                "context": uuid,
                "payload": payload
        };
        websocket.send(JSON.stringify(json));
    }
}

// saves a payload
function saveValues(payload) {
    if (websocket) {
        const json = {
                "event": "setSettings",
                "context": uuid,
                "payload": payload
        };
        websocket.send(JSON.stringify(json));
    }
}

function openUrl(url) {
    if (websocket) {
        const json = {
                "event": "openUrl",
                "payload": {
                    "url": url
                }
        };
        websocket.send(JSON.stringify(json));
    }
}