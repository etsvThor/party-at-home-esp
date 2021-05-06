# party-at-home-esp

## Install Platformio
To program the esp the platformio plugin in visual studio code(VSC) can be used or the CLI of Platformio.
[Download VSC] (https://code.visualstudio.com/download)
[Guide to install Platformio in VSC] (https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode)

## Upload and test connection
Upload it to the esp and connect to Thorpary access point and fill in your wifi credentials.
To test the connection the serial monitor can be opened and viewed to see if the esp is connected to the websocket.

## Testing the color changing
You can test the color changing by sending blue,red or green to the websocket.
This can be done by using a websocket [test chrome extension](https://chrome.google.com/webstore/detail/websocket-test-client/fgponpodhbmadfljofbimhhlengambbn)
Fill in the following in the url and press open:
```
wss://party-at-home.thor.edu:443/ws/chat/thorparty/
```
To change color send a request with the color: red , blue or green . 
