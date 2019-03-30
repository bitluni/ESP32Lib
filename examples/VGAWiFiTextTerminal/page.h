R"(
<html>

<head>
	<title>bitluni's VGA text terminal</title>
	<script>
		function load() {
			document.querySelector('#in').focus();
		}

		function send() {
			var e = document.querySelector('#in');
			var xhr = new XMLHttpRequest();
			xhr.open('POST', '/text');
			xhr.send(e.value);
			document.querySelector("#log").innerHTML = e.value + "\n" + document.querySelector("#log").innerHTML;
			e.value = '';
		}
	</script>
	<style>
		h1
			{
				color: white;
				background: #800000;
				padding: 10px;
				border-radius: 5px;
				margin-bottom: 5px;
			}
			body
			{
				font-family: monospace;
				padding: 0;
			}
			input, textarea, button
			{
				border-style: solid;
				border-radius: 3px;
			}
			td 
			{
				width: auto;
			}
			td.min 
			{
				width: 1%;
			}
			#in{
				margin-bottom: 10px;
				width: 100%;
			}
			#send{
				margin-bottom: 10px;
			}
			#log
			{
				resize: vertical;
				height: 200px;
				width: 100%
			}
			#area
			{
				margin-top: 100px;
				height: 500px;
			}
			table
			{
				width: 100%;
			}
		  
		</style>
</head>

<body onload='load()'>
	<h1>bitluni's VGA text terminal</h1>
	<table>
		<tr>
			<td><input id='in' type='text' tabindex=1 onchange='send()' autocomplete='off'></td>
			<td class='min'><button id='send' onclick='send()'>send</button></td>
		</tr>
		<tr>
			<td colspan=2>
				<textarea id='log' readonly>
					</textarea>
			</td>
		</tr>
	</table>

</body>

</html>

)"