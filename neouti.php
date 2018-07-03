<?php 

function getRequest($host){
	$curl = curl_init();
        curl_setopt_array($curl, array(
                CURLOPT_RETURNTRANSFER => 1,
                CURLOPT_URL => $host . $_SERVER['REQUEST_URI'],
                CURLOPT_USERAGENT => 'NeoUTI Request'
        ));
        $resp = curl_exec($curl);
        curl_close($curl);
        echo $resp . "<br/>";
}
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
	if(isset($_GET['alarm'])){
		if($_GET['alarm']){
			// // connect to the mysql database
			$link = mysqli_connect('localhost', 'root', '15352400', 'saci');
			mysqli_set_charset($link,'utf8');
			
			$sql = "
					INSERT INTO Customers (CustomerName, ContactName, Address, City, PostalCode, Country)
					VALUES ('Cardinal', 'Tom B. Erichsen', 'Skagen 21', 'Stavanger', '4006', 'Norway');
					";
			 
			// excecute SQL statement
			$result = mysqli_query($link,$sql);
			 
			// die if SQL statement failed
			if (!$result) {
			  http_response_code(404);
			  die(mysqli_error());
			}

			mysqli_close($link);
			echo "doiing";
		}
		getRequest("www.dweet.io/dweet/for");	
	}
}

//foreach($_GET as $key=>$value){
//    echo $key, ': ', $value, "<br/>";
//}
?>
