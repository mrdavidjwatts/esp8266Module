<?php
// getting the weather
if (empty($_GET["weather"])) {
    $weatherval = "2644668";
	// if no weather value is present add leicester as a default
}
else{
$weatherval = htmlspecialchars($_GET["weather"]);
}
$i = 0; // counter
$url = "http://open.live.bbc.co.uk/weather/feeds/en/" . $weatherval . "/observations.rss"; // url to parse
$rss = simplexml_load_file($url); // XML parser
function multiexplode ($delimiters,$string) {
    
    $ready = str_replace($delimiters, $delimiters[0], $string);
    $launch = explode($delimiters[0], $ready);
    return  $launch;
}
$exploded = "";
foreach($rss->channel->item as $item) {
if ($i < 1) { // parse only 10 items
  $exploded = multiexplode(array(",",".","|",":"),$item->title);
  //echo $exploded[3];
  
  //$wethval2 = explode(':', $wethval2[1],-1);
 // echo preg_replace('/[^a-zA-Z0-9_%\[().\]\\/-]/s', '', $exploded[3]);
  // 
    
 // = preg_replace('/[^a-zA-Z0-9_%\[().\]\\/-]/s', '', $wethval2[5]);
}

$i++;
}
$exploded[3] = preg_replace('/[^a-zA-Z0-9_%\[().\]\\/-]/s', '', $exploded[3]);
$exploded[2] = trim($exploded[2]);
// end getting weather

// getting value passed by the arduino
$val = htmlspecialchars($_GET["num"]);
// end getting value

// reading file from storage
$myfile2 = fopen("symbol.txt", "r") or die("Unable to open file!");
$symbol = fgets($myfile2);
fclose($myfile2);
$symbol2 = 0;
if($symbol < 5){
$symbol2 = $symbol + 1;	
}
else {
$symbol2 = 0;	
}
$myfile3 = fopen("symbol.txt", "w") or die("Unable to open file!");
fwrite($myfile3, $symbol2);
fclose($myfile3);
// end reading file

// writing files for storage
$myfile = fopen("newfile.txt", "a") or die("Unable to open file!");
date_default_timezone_set("Europe/London");
$txt = date("D M j G:i:s T Y \r\n");
$txt = $txt . "number is: " . $val ."\r\n";
fwrite($myfile, $txt);
fclose($myfile);
//end writing file

// printing out values and using start and end characters
echo "*+";
echo date("D M j");
echo "@";
echo date("H:i:s");
echo "|";
echo $symbol;
echo "$";
print_r ($exploded[2]); // weather description
echo "^";
print_r($exploded[3]); // weather temperature

echo "~";
echo "\n";
// end printing out characters
?>
