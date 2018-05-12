<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>PHP Test Page</title>
</head>
<body>
    <h1>PHP Test Page</h1>
    <hr>
    <ul>
    <?php
    for ($i = 0; $i < 3; $i++) {
        echo "<li>$i:list</li>";
    }
    ?>
    </ul>
    <h2>$_SERVER</h2>
    <code>
        <?php var_dump($_SERVER); ?>
    </code>
    <h2>$_GET</h2>
    <code>
        <?php var_dump($_GET); ?>
    </code>
    <h2>$_POST</h2>
    <code>
        <?php var_dump($_POST); ?>
    </code>

    <form action="/info.php" method="POST">
        name:<input name="abc">
        <button type="submit">POST</button>
    <form>
</body>
</html>
