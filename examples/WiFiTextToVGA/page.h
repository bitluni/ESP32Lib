R"(
<head>
<title>bitluni's VGA text terminal</title>
<script>
function sendText()
{
	var e = document.getElementById("text");
	var xhr = new XMLHttpRequest();
	xhr.open('POST', '/text');
	xhr.send(e.value);
	e.value = "";
}

</script>
<style>
  .menu
  {
    background-color: #eeeeee;
    padding: 1px;
    display: block;
    margin: 3px;
    padding: 5px;
    border-radius: 3px;   
  }
  h2
  {
    color: white;
    background: #800000;
    padding: 10px;
    border-radius: 5px;
    margin-bottom: 5px;
  }
</style>
</head>
<body style='font-family: arial'>
<h2>bitluni's VGA text terminal</h2>
<div>
  <div class='menu'>
      <input id='text' type='text' onchange='sendText()'>
  </div>
</div>
</body></html>
)"
