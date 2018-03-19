<html>
 <head>
 </head>
 <body>
<?php
  $codefn = getenv("DOCUMENT_ROOT") . "/wp-content/uploads/rlalg.c";
  unlink($codefn);
  unlink($flagfn);
  $fileLocation = $codefn;
  $file = fopen($fileLocation,"w");
  fwrite($file,urldecode($_SERVER['QUERY_STRING']));
  fclose($file);
?>
 </body>
</html>
