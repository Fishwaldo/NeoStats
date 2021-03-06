<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2//EN">
<html>
<!-- ------------------------------------------------------------ -->
<!--  Run Time Access                                             -->
<!--  Copyright (C) 2004 Robert W Smith (bsmith@linuxtoys.org)    -->
<!--                                                              -->
<!--   This program is distributed under the terms of the GNU     -->
<!--   LGPL.  See the file COPYING file.                          -->
<!-- ------------------------------------------------------------ -->
<head>
<title>Table View</title>
</head>
<body>
<?php
    // comment this out if your PHP includes postgresql client support
    dl("pgsql.so");
    $tbl    = htmlentities($_GET['table']);
    $offset = htmlentities($_GET['offset']);
    $nrows  = htmlentities($_GET['nrows']);
    print("<h3><center>$tbl</center></h3>\n");

    // Suppress Postgres error messages
    // error_reporting(error_reporting() & 0xFFFD);

    // connect to the database Change this to be your username/password
    $connection = pg_connect("host=localhost port=8888 user=blah password=blah");    
    if ($connection == "") { 
        printf("$s%s%s", "Unable to connect to application.<br>",
            "Please verify that the application is running and ",
            "listening on port 8888.<br>");
        exit();
    }

    // print each row in a table 
    print("<center><table border=3 cellpadding=4>\n");

    // Get and print column names
    $command = "SELECT name, flags FROM rta_columns WHERE table = $tbl";
    $result = pg_exec($connection, $command);
    if ($result == "") { 
        print("<p><font color=\"red\" size=+1>SQL Command failed!</p>");
        print("<p>Command: $command</p>\n");
        exit();
    }

    // A flag to say there's at least one editable column.
    $readonly = 2;    // "2" indicates a read-only column.  See rta.h
    print("<tr>\n");
    for($row = 0; $row < pg_NumRows($result); $row++)
    {
        $colname  = pg_result($result, $row, 0);
        $colflags = pg_result($result, $row, 1);
        print("<th>$colname</th>");
        $readonly = $readonly & $colflags;
    }
    if ($readonly == 0)
        print("<th>&nbsp;</th>\n");
    print("</tr>\n");
    pg_freeresult($result);

    // execute query 
    $command = "SELECT * FROM $tbl LIMIT 20 OFFSET $offset";
    $result = pg_exec($connection, $command);
    if ($result == "") { 
        print("<p><font color=\"red\" size=+1>SQL Command failed!</p>");
        print("<p>Command: $command</p>\n");
        exit();
    }
    for($row = 0; $row < pg_NumRows($result); $row++)
    {
        print("<tr>\n");
        for($field = 0; $field < pg_numfields($result); $field++)
        {
            print("<td>");
            print(pg_result($result, $row, $field));
            print("</td>\n");
        }
        // Add link to edit the row if editable.
        if ($readonly == 0)
            print("<td><a href=rta_edit.php?table=$tbl&row=$row>(edit)</a></td>");
        print("</tr>\n");
    }
    print("</table></center>\n");

    // Add link to next set of rows if needed
    if ($offset + 20 < $nrows)
    {
        $offset = $offset + 20;
        $nextcount = $nrows - $offset;
        if ($nrows - $offset > 20)
            $nextcount = 20;
        print("<p align=right><a href=rta_view.php?table=$tbl&offset=$offset");
        print("&nrows=$nrows>Next $nextcount rows &gt;</a></p>\n");
    }

    // free the result and close the connection 
    pg_freeresult($result);
    pg_close($connection);
?>
</body>
</html>
