<?php
require_once('../include.php');
require_once('../util.inc');
require_once('forum.inc');
doHeader('Forum');

/* sanitize variable */
$_GET['id'] = stripslashes(strip_tags($_GET['id']));

$sort_style = $_GET['sort'];
$filter_min = $_GET['filter'];

// TODO: Constant for default sort style and filter values.
if ($sort_style == NULL) {
    $sort_style = "date-old";
}
if ($filter_min == NULL || $filter_min < -2 || $filter_min > 2) {
    $filter_min = -2;
}

$thread = getThread($_GET['id']);
$thread->incView();

$forum = getForum($thread->forum);

$logged_in_user = get_logged_in_user(false);

$is_subscribed = false;

if ($logged_in_user) {
    $result = sql_query("SELECT * FROM subscriptions WHERE (userid = " . $logged_in_user->id . ") AND (threadid = " . $thread->id . ")");
    if ($result) {
        $is_subscribed = (sql_num_rows($result) > 0);
    }
}
?>
<form action="thread.php" method="get">
<input type="hidden" name="id" value=<?php echo $thread->id ?>>
<table width=100% cellspacing=0 cellpadding=0>
  <tr valign="bottom">
    <td align="left" style="border:0px">
      <p>
        <span class="title"><?php echo $thread->title ?></span>
        <br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a> -> <a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a>
      </p>

      <a href=reply.php?thread=<?php echo $thread->id ?>#input>Reply to this thread</a><br>
<?php
if ($is_subscribed) {
    echo "You are currently subscribed to this thread.  <a href=subscribe.php?action=unsubscribe&thread=$thread->id>Click here to unsubscribe</a>.";
} else {
    echo "<a href=subscribe.php?action=subscribe&thread=$thread->id>Subscribe to this thread</a>";
}
?>
</td>
<td align="right" style="border:0px">Sort / Filter
<select name="sort">
  <option <?php if ($sort_style == 'date-old') echo 'selected' ?> value="date-old">Oldest first</option>
  <option <?php if ($sort_style == 'date-new') echo 'selected' ?> value="date-new">Newest first</option>
  <option <?php if ($sort_style == 'rating-high') echo 'selected' ?> value="rating-high">Highest rated first</option>
</select> 
<select name="filter">
  <option <?php if ($filter_min == 2) echo 'selected' ?> value="2">"Very helpful" only</option>
  <option <?php if ($filter_min == 1) echo 'selected' ?> value="1">At least "helpful"</option>
  <option <?php if ($filter_min == 0) echo 'selected' ?> value="0">At least "neutral"</option>
  <option <?php if ($filter_min == -1) echo 'selected' ?> value="-1">At least "unhelpful"</option>
  <option <?php if ($filter_min == -2) echo 'selected' ?> value="-2">All Posts</option>
</select>
  <input type="submit" value="Sort">
</td>
</tr></table>
</form>
<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th style="width: 150px">Author</th>
			<th>Message</th>
		</tr>
		<?php show_posts($thread, $sort_style, $filter_min, true, true) ?>
	</table>
</p>
<p>
<a href=reply.php?thread=<?php echo $thread->id ?>#input>Reply to this thread</a><br>
</p>
<?php
doFooter();
?>
