<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Milkyway@home Data Plots</title>
<style type="text/css">
<!--
body {
	font: 100% Arial, sans-serif;
	background-image: url(srvr.gif);
	margin: 0; /* it's good practice to zero the margin and padding of the body element to account for differing browser defaults */
	padding: 0;
	text-align: center; /* this centers the container in IE 5* browsers. The text is then set to the left aligned default in the #container selector */
	color: #000000;
}
.oneColLiqCtrHdr #container {
	width: 80%;  /* this will create a container 80% of the browser width */
	background: #FFFFFF; /* the auto margins (in conjunction with a width) center the page */
	border: 1px solid #000000;
	text-align: left; /* this overrides the text-align: center on the body element. */
	margin-top: 20px;
	margin-right: auto;
	margin-bottom: 20px;
	margin-left: auto;
}
.oneColLiqCtrHdr #header {
	background: #DDDDDD;  /* this padding matches the left alignment of the elements in the divs that appear beneath it. If an image is used in the #header instead of text, you may want to remove the padding. */
	padding-top: 5px;
	padding-right: 10px;
	padding-bottom: 0;
	padding-left: 10px;
}
.oneColLiqRtHdr #navBox {
	background: #DDDDDD;
	margin-bottom: 20px;
}
.oneColLiqCtrHdr #header h1 {
	margin: 0; /* zeroing the margin of the last element in the #header div will avoid margin collapse - an unexplainable space between divs. If the div has a border around it, this is not necessary as that also avoids the margin collapse */
	padding: 10px 0; /* using padding instead of margin will allow you to keep the element away from the edges of the div */
}
.oneColLiqCtrHdr #mainContent {
	padding: 0 20px; /* remember that padding is the space inside the div box and margin is the space outside the div box */
	background: #FFFFFF;
}
.oneColLiqCtrHdr #footer { 
	padding: 0 10px; /* this padding matches the left alignment of the elements in the divs that appear above it. */
	background:#DDDDDD;
} 
.oneColLiqCtrHdr #footer p {
	margin: 0; /* zeroing the margins of the first element in the footer will avoid the possibility of margin collapse - a space between divs */
	padding: 10px 0; /* padding on this element will create space, just as the the margin would have, without the margin collapse issue */
}

ol#toc {
    height: 2em;
    list-style: none;
    margin: 0;
    padding: 0;
}

ol#toc li {
    background: #bdf url(tabs.gif);
	font-weight:bolder;
    float: left;
    margin: 0 1px 0 0;
    padding-left: 10px;
}

ol#toc li:hover{
    background-color: #48f;
    background-position: 0 -60px;
}

ol#toc a {
    background: url(tabs.gif) 100% 0;
    color: #FFF;
    float: left;
    line-height: 2em;
    padding-right: 10px;
    text-decoration: none;
}

ol#toc a:hover {
	background-position: 100% -60px;
    color: #fff;
    font-weight: bold;
}

ol#toc li.current {
    background-color: #48f;
    background-position: 0 -60px;
}

ol#toc li.current a {
    background-position: 100% -60px;
    color: #fff;
    font-weight: bold;
}

div.content {
    border: #48f solid 3px;
    clear: left;
    padding: 1em;
}
-->
</style></head>

<body class="oneColLiqCtrHdr">

<div id="container">
  <div id="header">
    <h1 align="center"><img src="mw.png" alt="milkyway@home" width="597" height="151" align="top" longdesc="file:///C|/Users/Brian/Desktop/milkyway@home/dl/mw.png" /></h1>
    <div id="navBox">
        <center>
        <ol id="toc">
            <span onmouseout="this.style.color = '#000000';">
            <li><a href="index.php">Home</a></li>
            <li><a href="join.php">Join</a></li>
            <li><a href="community.php">Community</a></li>
            <li><a href="information.php">Project Info</a></li>
            <li class="current"><a href="plots.php">Data Plots</a></li>
            <li><a href="science.php">Science</a></li>
            <li><a href="http://milkyway.cs.rpi.edu/milkyway/forum_index.php">Message Boards</a></li>
            <li><a href="http://milkyway.cs.rpi.edu/milkyway/home.php">Your Account</a></li>
            </span>
        </ol>
        </center>
    </div>
  <!-- end #header --></div>
  <div id="mainContent">
    <h1>Data Plots</h1>
    <hr />
    <h2><strong>Search Progress</strong></h2>
    <p><strong>Milkyway@home</strong> uses evolution-inspired search methods to find the optimal fit of models of the Milky Way galaxy to observed star data. By putting models of multiple stripes together we can create a highly accurate, three dimentional model of the Milky Way. For each search, the server tracks a population of individuals, each corresponding to a possible model of the Milky Way to a stripe of observed stars. The server then generates work-units which are combinations of different parent individuals in the population that result in the creation of child individuals. Then, the fitness of these children are calculated, returning the result. When a result is reported to the server that will improve the population, it is updated and that child individual becomes a parent. The following figures are updated every five minutes with the results. By running these evolutionary searches, we can find the best fit of our Milky Way models to the observed data. There is a thread in our forums discussing this in more detail <a href="http://milkyway.cs.rpi.edu/milkyway/forum_thread.php?id=643">here</a></p>
<div align="center">
      <?php
		$handle = opendir ('/export/www/boinc/milkyway/download/plots/');
		while (false !== ($file = readdir($handle))) {
			if($file != "." && $file != ".." && $file != "plots.php" && $file != basename(__FILE__)) {
				echo '<img src="http://milkyway.cs.rpi.edu/milkyway/download/plots/'.$file.'"/>';
				echo '<br><br>';
			}
		}
	?>
    </div>
    <!-- end #mainContent -->
  </h1>
  </div>
  <div id="footer">
  <p align="center">Copyright © 2010 AstroInformatics Group 
    <a href=http://boinc.berkeley.edu><img align=middle border=0 src=http://boinc.berkeley.edu/logo/pb_boinc.gif></a> </p>
  <!-- end #footer --></div>
<!-- end #container --></div>
</body>
</html>
