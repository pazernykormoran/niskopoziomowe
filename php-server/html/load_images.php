<div>
<?php

$files = scandir('photos/');
foreach($files as $file) {
  //do your work here

$parts=pathinfo($file);
if($parts['extension'] == "jpg"){
echo "<img style=\" float:left; \" width= \"200\" src=\" photos/$file\"> ";
}

}
?>
</div>
