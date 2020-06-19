function guarantee_stylesheet(){
 var thehead = document.getElementsByTagName('head')[0];
 if (document.getElementsByTagName('style').length < 1){
    var thesty = document.createElement('style');
    thesty.innerHTML = thesty.innerHTML + ' div#diag_kludge{color: #0099ff;} ';
    thesty.innerHTML = thesty.innerHTML + ' div#diag_kludge{background-color: #660000;} ';
    thehead.appendChild(thesty);
    divv = document.getElementById('diag_kludge');
    titl = headd.getElementsByTagName('title')[0];
    }
}
window.addEventListener('load', guarantee_stylesheet, false);
