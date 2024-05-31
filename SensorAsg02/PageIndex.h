const char CONFIG_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>

<head>
    <title>NodeMCU Control Panel</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
            color: #333;
        }

        .container {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
            max-width: 600px;
            margin: 0 auto;
        }

        h1,
        p {
            color: #333;
        }

        form {
            display: flex;
            flex-direction: column;
        }

        label {
            font-weight: bold;
            margin-top: 10px;
        }

        input[type='text'],
        input[type='password'] {
            width: 100%;
            padding: 10px;
            margin-top: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }

        input[type='radio'] {
            margin-right: 10px;
        }

        input[type='submit'] {
            background-color: #008CBA;
            color: white;
            padding: 10px 15px;
            margin-top: 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        input[type='submit']:hover {
            background-color: #005f87;
        }
    </style>
</head>

<body>
    <div class='container'>
        <h1>NodeMCU WiFi Control</h1>
        <p>Current SSID: <strong>%%SSID%%</strong></p>
        <p>Change your WiFi settings, Device ID, or Buzzer State:</p>
        <form method='get' action='setting'>
            <label for='ssid'>SSID:</label>
            <input type='text' id='ssid' name='ssid' maxlength='32' value='%%SSID%%' required>
            <label for='password'>Password:</label>
            <input type='password' id='password' name='password' maxlength='32' value='%%PASSWORD%%' required>
            <label for='deviceId'>Device ID:</label>
            <input type='text' id='deviceId' name='deviceId' maxlength='32' value='%%DEVICEID%%' required>
            <label for='buzzer'>Buzzer State:</label>
            <div>
                <input type='radio' name='buzzer' value='1' %%BUZZERON%%>On
                <input type='radio' name='buzzer' value='0' %%BUZZEROFF%%>Off
            </div>
            <input type='submit' value='Update Settings'>
        </form>
    </div>
</body>

</html>
)=====";




const char REBOOT_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Reboot</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
            color: #333;
        }

        .container {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
            max-width: 600px;
            margin: 0 auto;
            text-align: center;
        }

        h1,
        h2 {
            color: #333;
        }
    </style>
</head>

<body>
    <div class='container'>
        <h1>Success!</h1>
        <h2>Please reboot to take effect</h2>
    </div>
</body>

</html>
)=====";
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>Obstacle Detector System</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
            color: #333;
        }

        .container {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
            max-width: 600px;
            margin: 0 auto;
            text-align: center;
        }

        h1 {
            color: #333;
            margin-bottom: 20px;
        }

        p {
            color: #666;
            margin: 10px 0;
        }

        span {
            color: #f00;
            font-weight: bold;
        }

        #alert {
            color: #f00;
            font-weight: bold;
            margin-top: 10px;
        }

        form {
            margin-top: 20px;
        }

        label {
            color: #333;
            font-weight: bold;
        }

        input[type='number'] {
            padding: 10px;
            margin-top: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
            width: 100%;
            max-width: 200px;
        }

        input[type='submit'] {
            background-color: #008CBA;
            color: white;
            padding: 10px 15px;
            margin-top: 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        input[type='submit']:hover {
            background-color: #005f87;
        }
    </style>
    <script>
    function updateDistance() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                var distance = parseFloat(this.responseText);
                document.getElementById('%%DISTANCECM%%').innerText = distance;
                var rangeValue = parseFloat(document.getElementById('%%RANGEVALUE%%').value);
            }
        };
        xhttp.open('GET', '/distance', true);
        xhttp.send();
    }
    // Function to reload the page after a specified interval (in milliseconds)
    function autoReload() {
        setTimeout(function() {
            location.reload();
        }, 4000);  // Refresh every 4 seconds (adjust as needed)
    }

    // Call the autoReload function when the page loads
    window.onload = autoReload;


</script>

</head>

<body>
    <div class='container'>
        <h1>Obstacle Detection System</h1>
        <p>Detection Range (cm): <span>%%RANGEVALUE%%</span></p>
        <p>Distance (cm): <span>%%DISTANCECM%%</span></p>
         <p>Message: <span>%%MESSAGE%%</span></p>

        <form action='/update' method='GET'>
            <label for='range'>Detection Range (cm):</label><br>
            <input type='number' id='range' name='range' min='1' max='400' value='%%RANGEVALUE%%'><br>
            <input type='submit' value='Update Settings'>
        </form>
    </div>
</body>

</html>
)=====";
