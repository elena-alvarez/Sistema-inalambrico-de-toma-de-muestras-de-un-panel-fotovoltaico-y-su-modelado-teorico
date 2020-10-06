/*	TFM Elena Álvarez Castro
	Página web para la visualización de datos
	Calculo de parametros teoricos - Código javascript
 * 	Fichero metodos.js
 * 
 */

 ///////////////////////////

		var gmIV; //elemento gravico para dibujar

		var colores = ["red","blue", "yellow", "cyan", "magenta",
					  "orange","purple", "aqua", "aquamarine", "blueviolet", "brown", "chocolate"];
		var indiceColor=0;

		var corriente =[]; // Array con corrientes
		var tension   =[];
		var potencia  =[];

		//lineas de representacion de los metodos
		//Para visualizar.
		var lMetodoRW;
		var lMetodoX;
		var lMetodoV;
		var lMetodoS;

		q=1.6022e-19;

		function MetPrincipal() {
		  
			gmIV = document.getElementById("IV");
		}

		function nuevosDatosF() {
		  if (muestrasF.length == 0){
			var xhttp = new XMLHttpRequest();
			xhttp.onreadystatechange = function() {
			 if (this.readyState == 4 && this.status == 200) {
			  var myArr = JSON.parse(this.responseText);
			  
			  MetCreaNuevaCurva(myArr);
			 }
		   };
		   xhttp.open("GET", "nDatosFloat.txt", true);
		   xhttp.send();
		 }
		 else
		  MetCreaNuevaCurva(muestrasF);
		}

		function MetCreaNuevaCurva(arr) {
		   
		   muestrasF = arr; // muestrasF es global
		   // calculamos la potencia de cada muestra
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

		   //Imp y Vmp ya estan calculado en el bucle anterior.
		   valores=MetodoRW(Isc,Voc,Imp,Vmp);
		   vTeoricos=CurvaTeorica(valores.Ig,valores.A,valores.Isat,valores.Rs,valores.Rp,Voc,"red");
		   lMetodoRW = DibujaLineasGrafica(vTeoricos.Imetodo,vTeoricos.Vmetodo,"red","RW");


		   valores = MetodoXiao(Isc,Voc,Imp,Vmp);
		   vTeoricos=CurvaTeorica(valores.Ig,valores.A,valores.Isat,valores.Rs,valores.Rp,Voc,"blue");
		   lMetodoX = DibujaLineasGrafica(vTeoricos.Imetodo,vTeoricos.Vmetodo,"blue","Xiao");

		   valores = MetodoVillalva(Isc,Voc,Imp,Vmp);
		   vTeoricos=CurvaTeorica(valores.Ig,valores.A,valores.Isat,valores.Rs,valores.Rp,Voc,"green");
		   lMetodoV = DibujaLineasGrafica(vTeoricos.Imetodo,vTeoricos.Vmetodo,"green","Villalva");
		  
		   valores = MetodoSilva(Isc,Voc,Imp,Vmp);
		   vTeoricos=CurvaTeorica(valores.Ig,valores.A,valores.Isat,valores.Rs,valores.Rp,Voc,"black");
		   lMetodoS = DibujaLineasGrafica(vTeoricos.Imetodo,vTeoricos.Vmetodo,"black","Villalva");

		}

		function MetodoRW(Isc,Voc,Imp,Vmp){
			var valoresFormula = new Object();
			Iph=Isc;
			numeradorVt  =(2*Vmp-Voc)*(Isc-Imp);
			denominadorVt=Isc+((Isc-Imp)*Math.log(1-(Imp/Isc)));
			Vt=numeradorVt/denominadorVt;
			Io=Isc*Math.exp(-Voc/Vt);
			terminoLog = Math.log(1-(Imp/Isc));
			Rs=(Vt*terminoLog)+Voc-Vmp;
			Rs=Rs/Imp;

			Ig=Iph;
			Isat=Io;
			T=302;
			K=1.38065e-23; //Constante de Bolzam
			Ns=36 //Numero de celdas en serie
			q=1.6022e-19;
			A=Vt*q/(Ns*K*T);
			//llama a curva teorica
			var Rp=Infinity; // Estimado
			actualizaTablaMetods("Rw",Ig,A,Isat,Rs,Rp);
			valoresFormula.metodo = "Rw";
			valoresFormula.Ig=Ig;
			valoresFormula.A = A;
			valoresFormula.Isat = Isat;
			valoresFormula.Rs = Rs;
			valoresFormula.Rp=Rp;
			return(valoresFormula);
			//CurvaTeorica(Ig,A,Isat,Rs,Rp,Voc);
		}

		function MetodoSilva(Isc, Voc, Imp, Vmp){
			var valoresFormula = new Object();
			var matrizMAEP    = [];
			var valoresMetodo = [];

			T=302;
			K=1.38065e-23;
			Ns = 36;
			q= 1.6022e-19;

			Ig= Isc;
			columnaMaep = 1;
			indiceMaep  = 0;
			for (A=1; A<2.5; A=A+0.5){
				for (Rs=0.5; Rs<15; Rs=Rs+0.5){
					resistenciaSerie=Rs;
					Vt=(Ns*A*K*T)/q;
					RpTerminoExp=(Math.exp((Vmp+Imp*Rs)/Vt)-1)/(Math.exp(Voc/Vt)-1);
					RpNumerador=(Voc*RpTerminoExp)-Vmp-(Imp*Rs);
					RpDenominador=Imp+Ig*(RpTerminoExp-1);
					Rp=RpNumerador/RpDenominador;
					if(Rp<0){
					  Rp=Rp;
					  continue;
					}
					Isat = Ig-(Voc/Rp);
					Isat = Isat/(Math.exp(Voc/Vt)-1);
					//Calculamos un array con tensiones y corrientes según formulas
					//Tomamos las tensiones de las muestras
					
					for (i=0; i<muestrasF.length; i++){
					   valoresMetodo[i]=new Object();
					   valoresMetodo[i].v=muestrasF[i].v;
					   valoresMetodo[i].i=NaN;
					}
					//Calculamos la intensidad asociada a cada tension según formula
					valoresMetodo[0].i=Isc;
					for (i=1; i<valoresMetodo.length; i++){
						Ip=valoresMetodo[i-1].i;
						while(isNaN(valoresMetodo[i].i)){
							Inew = Ig -Isat * (Math.exp((valoresMetodo[i].v+Ip*Rs)/Vt)-1)-
							  (valoresMetodo[i].v+Ip*Rs)/Rp;
							if (Inew>Ip)
							   valoresMetodo[i].i=0;
							else{
								actualError = Ip - Inew;
								if (actualError < 0.0001)
								  if(Inew < 0)
									 valoresMetodo[i].i = 0;
								  else
									 valoresMetodo[i].i = Inew;
							} 
							Ip = Ip - 0.00001;
						}
					}
					//valoresMetodo=CurvaTeorica(Ig,A,Isat,Rs,Rp,Voc,"green");
					for(i=0; i<valoresMetodo.length;i++){
						valoresMetodo[i].p = valoresMetodo[i].i*valoresMetodo[i].v;
					}
					for(i=0,MAEP=0; i<valoresMetodo.length;i++){
						valoresMetodo[i].error = valoresMetodo[i].p-muestrasF[i].p;
						valoresMetodo[i].error = Math.abs(valoresMetodo[i].error);
						MAEP += valoresMetodo[i].error;
					}
					MAEP=MAEP/valoresMetodo.length;

					//grabamos en la matriz maep los valores actuales.

					matrizMAEP[indiceMaep] = new Object();
					matrizMAEP[indiceMaep].Maep = MAEP;
					matrizMAEP[indiceMaep].a = A;
					matrizMAEP[indiceMaep].Rs = Rs;
					matrizMAEP[indiceMaep].Rp = Rp;
					matrizMAEP[indiceMaep].Ig = Ig;
					matrizMAEP[indiceMaep].Isat = Isat;
					indiceMaep++;
				}
			}
			// Ordenamos la matriz MAEP
			matrizMAEP.sort(function(a,b){return a.Maep - b.Maep});


			valoresFormula.metodo = "S";
			valoresFormula.Ig   =matrizMAEP[0].Ig;
			valoresFormula.A    =matrizMAEP[0].a;
			valoresFormula.Isat =matrizMAEP[0].Isat;
			valoresFormula.Rs   =matrizMAEP[0].Rs;
			valoresFormula.Rp   =matrizMAEP[0].Rp;
			actualizaTablaMetods("S",valoresFormula.Ig,valoresFormula.A,valoresFormula.Isat,valoresFormula.Rs,valoresFormula.Rp);

			return(valoresFormula);

		}
		function MetodoVillalva(Isc,Voc,Imp,Vmp){
			var valoresFormula = new Object();

			T=302;
			K=1.38065e-23; //Constante de Bolzam
			Ns=36; //Numero de celdas en serie
			q=1.6022e-19;

			A=1.3;
			Ig=Isc;
			Vt=(Ns*A*K*T)/q;
			Isat = Ig/(Math.exp(Voc/Vt)-1);
			//Proponemos
			Rs=0;
			Rpmin=(Vmp/(Isc-Imp))-((Voc-Vmp)/Imp);
			Rp=Rpmin;

			pMaxMuestra=Imp*Vmp; //Potencia maxima de la muestra

			//INicio del bucle
			errorPmax = 100;
			while ((errorPmax > 0.005) && (Rs<50)){
				Ig=Isc;
				//solve eq 15
				exponenteIsat=(Vmp+(Rs*Imp))/Vt;
				RpNumerador  =Vmp*(Vmp+Rs*Imp);
				RpDenom=Vmp*(Ig-Isat*(Math.exp(exponenteIsat)-1));
				RpDenom=RpDenom-pMaxMuestra;
				Rp=RpNumerador/RpDenom;

				if(Rp<0){
					Rs=Rs+0.1;
					continue;
				}
				//solve eq 16
				Ig=(Rp+Rs)*Isc/Rp;
				valoresTeoricos=CurvaTeorica(Ig,A,Isat,Rs,Rp,Voc,"black");
				pMaxTeorica=0;
				for (i=0;i<valoresTeoricos.Imetodo.length;i++){
					potencia=valoresTeoricos.Imetodo[i]*valoresTeoricos.Vmetodo[i];
					if (potencia>pMaxTeorica)
						pMaxTeorica=potencia;
				}
				errorPmax=Math.abs(pMaxTeorica-pMaxMuestra);
				Rs=Rs+0.1;
			}

			actualizaTablaMetods("V",Ig,A,Isat,Rs,Rp);
			valoresFormula.metodo = "V";
			valoresFormula.Ig=Ig;
			valoresFormula.A = A;
			valoresFormula.Isat = Isat;
			valoresFormula.Rs = Rs;
			valoresFormula.Rp=Rp;
			return(valoresFormula);
		}
		function MetodoXiao(Isc,Voc,Imp,Vmp){
			/* Metodo de cuatro parámetros. Desprecia el efecto de la resistencia
			 * en paralelo.
			 * Se basa en que en un modulo PV la derivada de la potencia respecto
			 * al voltaje es cero en el punto de máxima potencia.
			 * */
			var valoresFormula = new Object();
			var Vmetodo = [];
			var Imetodo = [];
			var VIPmetodo = []; //tension,corriente,potencia v,i,p
			var errorMatriz = [];

			T=302;
			K=1.38065e-23; //Constante de Bolzam
			Ns=36; //Numero de celdas en serie
			var Ig,Isat,A,Rs,Rp;

			Ig=Isc;

			for (i = 0; i< muestrasF.length; i++){
				Vmetodo[i] = muestrasF[i].v; //tomamos las tensiones
			}
			Vmetodo.sort(function(a, b){return a - b}); //ordenamos tensiones

			indiceError = 0;
			for (A=1.0; A<2; A+=0.05){
				Vt   =(Ns*A*K*T)/q; //Eq 2
				Isat = Ig/(Math.exp(Voc/Vt)-1);
				Rs=(Vt*Math.log(((1-Imp/Ig)*Math.exp(Voc/Vt)+Imp/Ig))-Vmp)/Imp;

				index = 0;
				Imetodo[index] = Ig;
				for (j=0; j < Vmetodo.length; j++){
					V  = Vmetodo[j];
					if (index == 0)
						Ip = Ig;
					else
						Ip = Imetodo[index-1];
					while(Imetodo.length < index+1){
						Inew=Ig-Isat*(Math.exp((V+Ip*Rs)/Vt)-1);
						if (Inew > Ip)
							Imetodo[index] = 0;
						else{
							actualError=Ip-Inew;
							if(actualError < 0.0001)
								if (Inew < 0)
									Imetodo[index]=0;
								else
									Imetodo[index]=Inew;
						}
						Ip = Ip - 0.00001;
					}//busqueda de Imetod(V)
					index++;
				}
				for (i = 0; i<Vmetodo.length; i++){
					var vip = {};
					vip.v = Vmetodo[i];
					vip.i = Imetodo[i];
					vip.p = Vmetodo[i] * Imetodo[i];

					VIPmetodo[i] = vip;
				}
				//Ordenamos por potencia  decreciente el array de VIP
				VIPmetodo.sort(function(a,b){return b.p - a.p});

				exponente = (VIPmetodo[0].v+VIPmetodo[0].i*Rs)/Vt;
				numerador = (Isat/Vt)*Math.exp(exponente);
				denominador = 1+(Isat*Rs/Vt)*Math.exp(exponente);
				t1=numerador/denominador;
				t2=VIPmetodo[0].i/VIPmetodo[0].v
				errorTotal = t2-t1;

				var objError = {};
				objError.error = errorTotal;
				objError.A     = A;

				errorMatriz[indiceError]=objError;
				errorMatriz[indiceError]=objError;
				indiceError++;
			} //bucle A
			//ordenamos de menor a mayor la matriz de errores
			errorMatriz.sort(function(a,b){return a.error - b.error});

			Aoptimo = errorMatriz[0].A;

			A  = Aoptimo;
			Vt = (Ns*Aoptimo*K*T)/q;
			Isat = Ig/(Math.exp(Voc/Vt)-1);
			Rs   = (Vt*Math.log(((1-Imp/Ig)*Math.exp(Voc/Vt)+Imp/Ig))-Vmp)/Imp;

			var Rp=Infinity; // Estimado
			actualizaTablaMetods("X",Ig,A,Isat,Rs,Rp);
			valoresFormula.metodo = "X";
			valoresFormula.Ig=Ig;
			valoresFormula.A = A;
			valoresFormula.Isat = Isat;
			valoresFormula.Rs = Rs;
			valoresFormula.Rp=Rp;
			return(valoresFormula);
			//CurvaTeorica(Ig,A,Isat,Rs,Rp,Voc);

		}

		function CurvaTeorica(Ig=0.32,A=1.5000,Isat=8.3e-7,Rs=15,Rp=1000000,Voc=20,colorCurva){

			var valoresCurva = new Object();
			var Vmetodo = []; //array con Tensiones
			var Imetodo = [];

			var T=302; //Temperatura
			var K=1.38065e-23; //Constante de Bolzam
			var Ns=36; //Numero de celdas en serie
			var q=1.6022e-19; // Carga del electron

			Vt=Ns*A*K*T/q;

			for (j=0; j<Voc+1; j++){
				Vmetodo[j]= j;
			}

			Imetodo[0] = Ig;
			var index=0;
			for (j=0; j<Vmetodo.length; j++){
				V =Vmetodo[j];
				if (index == 0)
					Ip = Ig;
				else
					Ip = Imetodo[index-1];
				while(Imetodo.length<(index+1)){
				   Inew=Ig-Isat*(Math.exp((V+Ip*Rs)/Vt)-1)-(V+Ip*Rs)/Rp;
				   if (Inew>Ip)
					Imetodo[index]=Ip;
				   else{
					actualError=Ip-Inew;
					if(actualError < 0.0001)
						if (Inew < 0) Imetodo[index] = 0;
						else Imetodo[index] = Inew
					Ip = Ip -0.00001;
				   }	
				}
				index ++;
			}
			valoresCurva.Imetodo = Imetodo;
			valoresCurva.Vmetodo = Vmetodo;
			return(valoresCurva);
			//DibujaLineasGrafica(Imetodo,Vmetodo,colorCurva);
		}
		function DibujaLineasGrafica(iFloat, vFloat,colorCurva,idMetodo){
			//agrupamos elementos
			gIV = document.getElementById("IV");
			var newG = document.createElementNS("http://www.w3.org/2000/svg", "g");
			newG.setAttribute('id', idMetodo);
			//newG.setAttribute("fill", colorCurva);
			
			var puntosPolilinea="";
			//Pasamos los arrays de corriente y tension a pixels.
			for (var i=0; i<iFloat.length; i++){
				yPixel = iFloat[i]*2000; //(640/0.32)
				yPixel = 670-yPixel;
				xPixel = vFloat[i]*(880.0/22.0);
				xPixel +=60;

				var newCirculo = document.createElementNS(
					"http://www.w3.org/2000/svg", 'circle'); //       
				newCirculo.setAttribute("fill",colorCurva); 
				newCirculo.setAttribute("cx",xPixel); 
				newCirculo.setAttribute("cy",yPixel); 
				newCirculo.setAttribute("r",2); //Set path's data
				newG.appendChild(newCirculo);
				puntosPolilinea += xPixel + ","+ yPixel+ " ";
			}
			var newPolilinea = document.createElementNS(
					"http://www.w3.org/2000/svg", 'polyline'); //       
			newPolilinea.setAttribute("points",puntosPolilinea); 

			newPolilinea.setAttribute("fill","none");
			newPolilinea.setAttribute("stroke-width","2");
			newPolilinea.setAttribute("stroke",colorCurva);
			newG.appendChild(newPolilinea);
			gIV.appendChild(newG);

			return (newG);
		}

		function actualizaTablaMetods(metodo,Ig,A,Isat,Rs,Rp){
			document.getElementById("ig"+metodo).innerHTML   = Ig.toPrecision(2);
			document.getElementById("a"+metodo).innerHTML    = A.toPrecision(3);
			document.getElementById("is"+metodo).innerHTML   = Isat.toExponential(2);
			document.getElementById("rs"+metodo).innerHTML   = Rs.toPrecision(3);
			document.getElementById("rp"+metodo).innerHTML   = Rp.toPrecision(3);
		}
		function VerOcultarLineaMetodo (nombreMetodo){
			var checkBox = document.getElementById(nombreMetodo);
			var lMetodo;
			switch (nombreMetodo) {
				case "verRw": lMetodo = lMetodoRW; break;
				case "verX": lMetodo = lMetodoX; break;
				case "verS": lMetodo = lMetodoS; break;
				case "verV": lMetodo = lMetodoV; break;
			}
			if (checkBox.checked == true)
				lMetodo.style.visibility = "visible";
			  else 
				lMetodo.style.visibility = "hidden";
			  //lMetodoRW.style.visibility = "hidden"
		}