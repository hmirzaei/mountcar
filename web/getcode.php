<html>
 <head>
 </head>
 <body>
<?php
  $codefn = getenv("DOCUMENT_ROOT") . "/wp-content/uploads/rlalg.c";
  $fileLocation = $codefn;
  $file = fopen($fileLocation,"w");
  fwrite($file,urldecode($_SERVER['QUERY_STRING']));
  fclose($file);
  $command = escapeshellcmd('./runalg.py');
  shell_exec($command);
?>
 </body>
</html>
