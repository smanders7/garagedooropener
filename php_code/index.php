<?php

require_once('GPIO.php');
$gpio = new GPIO();

define("RELAY_DELAY", 1);

define("PASSWORD", "YourPasswordHere");
define("PASSWORD_HEX_SIZE", 32);
define("PASSWORD_SIZE", 16);
define("AES256_CRYPTO_KEY_SIZE", 32);
define("CHALLENGE_TOKEN_SIZE", 16);
define("PATH_TO_AES256_DECRYPT", "/var/www/garagedoor/aes256_decrypt");

$status_pins = array ( 9 );              // single device at pin #9
//$status_pins = array (2, 3, 4, 5);     // select if using DFRobot RelayShield
//$status_pins = array (2, 3 );          // two devices at pins #2 and #3
//$status_pins = array (2, 3, 4, ... );  // even more devices at pins #2, #3, #4, etc ...

session_name('GLOBAL'); 
session_id('TEST'); 
session_start(); 
$currentChallengeToken = $_SESSION['token'];
session_write_close(); 

$fp = fopen("debugging.out","a");

if (array_key_exists('password',$_GET)) {
  $encrypted_password = $_GET['password'];

  // relayPin must be then present
  if (array_key_exists('relayPin',$_GET)) {
    $r_pin = $_GET['relayPin'];
  }

  // Validate Password
  $cmd = PATH_TO_AES256_DECRYPT . " -k " . $currentChallengeToken . " -d " . $encrypted_password;
  $o_decrypted = exec($cmd);

  if (strncmp($o_decrypted, PASSWORD, PASSWORD_SIZE) == 0) {

    fwrite($fp,"Password Matched\n");

    // Toggle Relay
    $gpio->output($r_pin, 0);
    sleep(RELAY_DELAY);
    $gpio->output($r_pin, 1);
  }
}

// write opening XML element to output stream
echo "<?xml version=\"1.0\"?>\n";
echo "<myDoorOpener>\n";

// write current door status
//for ($i = 0; i < sizeof(statusPins); i++)
foreach ($status_pins as $s_pin)
{
  echo "<status statusPin=\"";
  echo "$s_pin";
  echo "\">";

  // write current open/close state to output stream
  echo (isOpen($gpio, $s_pin) == 0 ? "Opened": "Closed");
  echo "</status>\n";
}
 
// re-generate new challenge token
$currentChallengeToken = "Cyber" . date('H') . date('i') . date('s');

fwrite($fp,"New currentChallengeToken: " . $currentChallengeToken . "\n");

session_name('GLOBAL'); 
session_id('TEST'); 
session_start(); 
$_SESSION['token'] = $currentChallengeToken; 
session_write_close(); 

// write challenge token to output stream
echo "<challengeToken>";
echo "$currentChallengeToken";
echo "</challengeToken>\n";

// write closing XML element to output stream
echo "</myDoorOpener>\n";

function isOpen($gpio, $s_pin)
{
  return $gpio->input($s_pin);
}

fwrite($fp,"Closing\n\n");
fclose($fp);

?>
