<html>
 <head>
 </head>
 <body>
<?php
  $command = escapeshellcmd('./stop.py');
  shell_exec($command);
?>
 </body>
</html>
