/*	TFM Elena Álvarez Castro
	Página web para la visualización de datos
	Extracción de datos y representación gráfica de la Curva I-V - Código javascript
 * 	Fichero iv.js
 * 
 */
		var gIV;
		var temperaturaCentigrados;
		var muestrasF =[]; //array de muestras en coma flotante
						   //cada elemento tiene v,i,p
						   //Tension, corriente, potencia calculada

		var colores = ["red","blue", "yellow", "cyan", "magenta",
					  "orange","purple", "aqua", "aquamarine", "blueviolet", "brown", "chocolate"];
		var indiceColor=0;
		function CambiaTamanioGrafica(grSvg){
						grSvg.setAttribute("width", 300);
						grSvg.setAttribute("height", 400);
		}
		function Principal() {
		  
			gIV = document.getElementById("IV");
				gIV.setAttribute("padding-top","20%");
				//gIV.setAttribute("width", 1000);
				//gIV.setAttribute("height", 700);
			
			CreaLineasEscala();
			// Pedimos una curva inicial
			nuevosDatos();
		}
		function CreaLineasEscala(){
			var contLineaScala=document.getElementById("lineasEscala");

			for (var i=100; i<980; i+=40){
			  var newLinea = document.createElementNS("http://www.w3.org/2000/svg", 
					  'line');  
			  newLinea.setAttribute("x1",i);
			  newLinea.setAttribute("y1",670);
			  newLinea.setAttribute("x2",i);
			  newLinea.setAttribute("y2",30);
				  contLineaScala.appendChild(newLinea);
			}
			for (var i=30; i<651; i+=20){
			  var newLinea = document.createElementNS("http://www.w3.org/2000/svg", 
					  'line');  
			  newLinea.setAttribute("x1",60);
			  newLinea.setAttribute("y1",i);
			  newLinea.setAttribute("x2",940);
			  newLinea.setAttribute("y2",i);
				  contLineaScala.appendChild(newLinea);
			}
			for (var i=60,j=0; i<940; i+=40, j++) {
			  var newTexto = document.createElementNS("http://www.w3.org/2000/svg", 
					  'text');  
			  newTexto.setAttribute("x",i);
			  newTexto.setAttribute("y",690);
			  newTexto.textContent= j.toString();
				  gIV.appendChild(newTexto);
			}
			
			for (var i=670, j=0.00; i>31; i-=40, j=j+0.02000) {
			  var newTexto = document.createElementNS("http://www.w3.org/2000/svg", 
					  'text');  
			  newTexto.setAttribute("x",20);
			  newTexto.setAttribute("y",i);
			  newTexto.textContent= j.toFixed(2);
				  gIV.appendChild(newTexto);
			}
			
		}
		function nuevosDatos() {
		  var xhttp = new XMLHttpRequest();
		  xhttp.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
			  //document.getElementById("demo").innerHTML +=
			  //this.responseText;
			  var myArr = JSON.parse(this.responseText);
			  temperaturaCentigrados = myArr.temperatura;
			  if (temperaturaCentigrados===undefined) //Ojo
				 temperaturaCentigrados = 23;
			  CreaNuevaCurva(myArr.muestras);
			}
		  };
		  xhttp.open("GET", "nDatosFloat.txt", true);
		  xhttp.send();
		}

		//Historico de datos con iluminacion Natural
		function nuevosDatosHistorico(nombreFichero) { 
			var xhttp = new XMLHttpRequest();
			xhttp.onreadystatechange = function() {
			  if (this.readyState == 4 && this.status == 200) {
				var myArr = JSON.parse(this.responseText);
				temperaturaCentigrados = myArr.temperatura;
				if (temperaturaCentigrados === undefined)
				  temperaturaCentigrados = 23;
				CreaNuevaCurva(myArr);
			  }
			};
			nuevoNombre = nombreFichero;
			xhttp.open("GET", nuevoNombre, true);
			xhttp.send();
		}
		  
		function LimpiaDatos(){

		  gIV = document.getElementById("IV");
		  var listaHijos=gIV.getElementsByTagName("circle");
		 

		  for (var i = 0; i < listaHijos.length; i++) {
			listaHijos[i].remove();
		  }
		}

		function CreaNuevaCurva(arr) {
		   muestrasF=arr;
		   //ordenamos el array
		   muestrasF.sort(function(a,b){
			  return(a.v-b.v);
		   });
		   //Calculo de valores experimentales para poner en la tabla
		   for(i = 0, pMax=0, iMax=0, vMax=0; i < muestrasF.length; i++){
			muestrasF[i].p=muestrasF[i].v*muestrasF[i].i;
			corriente[i]  =muestrasF[i].i;
			tension[i]    =muestrasF[i].v;
			potencia[i]   =muestrasF[i].v*muestrasF[i].i;
			if (potencia[i] > pMax){
				pMax=potencia[i];
				Imp =muestrasF[i].i;
				Vmp =muestrasF[i].v;
			}
			if (corriente[i]>iMax) iMax = corriente [i];
			if (tension[i]>vMax)   vMax = tension [i];
		   }
		   Isc  = iMax;
		   Voc  = vMax;

		   document.getElementById("Isc").innerHTML   = Isc.toPrecision(2);
		   document.getElementById("Voc").innerHTML   = Voc.toPrecision(2);
		   document.getElementById("Imp").innerHTML   = Imp.toPrecision(2);
		   document.getElementById("Vmp").innerHTML   = Vmp.toPrecision(2);
		   document.getElementById("T").innerHTML     = temperaturaCentigrados;
		   document.getElementById("Pm").innerHTML     = pMax.toPrecision(3);


		   //
		   for(i = 0; i < arr.length; i++){
			   yPixel = arr[i].i*2000; //(640/0.32)
			   yPixel = 670-yPixel;
			   xPixel = arr[i].v*(880.0/22.0);
			   xPixel +=60;
			var newCirculo = document.createElementNS("http://www.w3.org/2000/svg", 'circle'); //       
			newCirculo.setAttribute("fill",colores[indiceColor]); 
			newCirculo.setAttribute("cx",xPixel); 
			newCirculo.setAttribute("cy",yPixel); 
			newCirculo.setAttribute("r",2); //Set path's data
			gIV.appendChild(newCirculo);
		   }
		   indiceColor++;
		   if (indiceColor==colores.length) indiceColor = 0;
		}

		//Control de nivel iluminación artificial

		function ActualizaLuz(){
			var nivel = document.getElementById("barraLuz").value;
			var xhttp = new XMLHttpRequest();
			xhttp.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
			  var respuesta = JSON.parse(this.responseText);
			}
		  };
		  var url="nL.txt?nv="+nivel;
		  xhttp.open("GET", url, true);
		  xhttp.send();
		}