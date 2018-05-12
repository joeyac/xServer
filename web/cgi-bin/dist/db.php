<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Document</title>
</head>
<body>
<?php
$pdo = new PDO('sqlite:db/database.sqlite');
$selectQuery = "SELECT * FROM post";
$sth = $pdo->prepare($selectQuery);
$sth->execute();
$posts = $sth->fetchAll();

echo '<ul>';
foreach ($posts as $p) {
    echo '<li>' . $p['id'] . ':' . $p['content'] . ' by ' . $p['user'] . '</li>';
}
echo '</ul>';

?>
<form action="insert.php" method="post">
    content:<input type="text" name="content" required>
    user: <input type="text" name="user" required>
    <button type="submit">POST</button>
</form>
</body>
</html>

