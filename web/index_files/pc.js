!function(){function e(e){return I.baseUrl+e}function i(){return!!window.s_session}function t(){var e=navigator.userAgent;return!(-1!==e.indexOf("MSIE 6.0")||-1!==e.indexOf("MSIE 7.0")||-1!==e.indexOf("MSIE 8.0"))}function n(){var e=navigator.userAgent;return!(-1!==e.indexOf("MSIE 9.0"))}function o(e){var i={page_id:(new Date).getTime(),oper_act:"earth_day_egg_pc",refer:"",fm:e.fm||"click"};$.extend(i,e),ns_c(i)}function a(e){clearInterval(I.timer),clearInterval(I.videoTimer),z.mainContainer.remove(),z.playerOuter.remove(),e&&(z.doodle.remove(),$('img[usemap="#mp"]').hide(),$('img[usemap="#mp"]').css("visibility","hidden")),z={},z.videoP=1,$("#form").removeAttr("style")}function r(){var t={};t=i()?{position:"absolute",left:"50%","margin-left":"-135px",display:"inline-block",width:I.doodleW+"px",height:I.doodleH+"px",overflow:"hidden"}:{position:"relative",display:"inline-block",width:I.doodleW+"px",height:I.doodleH+"px",overflow:"hidden"};var n=i()?_.framesWhite:_.frames;return z.doodle=$('<div class="doodle" id="doodle"><img width="'+I.doodleW+'" heigth="'+I.doodleH+'" style="top:0;position: absolute;left: 0" src="'+e(n)+'"></div>').css(t).data("clicked",!1),z.doodle}function d(){$("#lg").append(r()),$('img[usemap="#mp"]').hide(),$('img[usemap="#mp"]').css("visibility","hidden"),$("#doodle").attr({width:270,height:129,usemap:"",title:I.doodleHoverText}).css("cursor","pointer").click(function(){if(o({rsv_act:"doodle"}),$(this).data("clicked")||!t())t()||$("map[name=mp]").find("area").click(),window.open($("map[name=mp]").find("area").attr("href"),"_target");else{z.mainContainer&&a();var n=i()?_.doodlePngW:_.doodlePng;$(this).data("clicked",!0).find("img").attr("src",e(n)),t()&&(c(),x())}}).hover(function(){$(this).attr("title",I.doodleHoverText)}),$("#form").css("z-index",I.zIndex+10)}function s(){var e='<style id="egg-style">.circle-1{position: absolute;width: 70px;height: 70px;left: 50%;margin-left: -26px;top: 50%;margin-top: -26px;border-radius: 50%;opacity: 0;border: 2px solid rgba(255,255,255,1);  animation: circle1 2s 0.5s infinite;-webkit-animation: circle1 2s 0.5s infinite;}@keyframes circle1{0%{transform: scale(0.5);opacity: 1}80%{transform: scale(1.2);opacity: 0}100%{transform: scale(0.5);opacity: 0}}@-webkit-keyframes circle1{0%{transform: scale(0.5);opacity: 1}80%{transform: scale(1.2);opacity: 0}100%{transform: scale(0.5);opacity: 0}}.circle-2{position: absolute;width: 40px;height: 40px;  left: 50%;  margin-left: -12px;  top: 50%;margin-top: -12px;  border-radius: 50%;  opacity: 0;  border: 2px solid rgba(255,255,255,0.9);  animation: circle1 2s 0.6s infinite;  -webkit-animation: circle1 2s 0.6s infinite;}.circle-3{  position: absolute;  width: 16px;  height: 16px;  left: 50%;  margin-left: -2px;  top: 50%;  margin-top: -2px;  border-radius: 50%;  opacity: 0;  border: 4px solid rgba(255,255,255,1);  filter: blur(1px);  -webkit-filter: blur(1px);    animation: circle3 2s infinite;  -webkit-animation: circle3 2s infinite;  }@keyframes circle3{  0%{transform: scale(1.3);opacity: 0}  70%{transform: scale(0.6);opacity: 1}  80%{transform: scale(0.65);opacity: 0}  100%{transform: scale(1.3);opacity: 0}}@-webkit-keyframes circle3{  0%{transform: scale(1.3);opacity: 0}  70%{transform: scale(0.6);opacity: 1}  80%{transform: scale(0.65);opacity: 0}  100%{transform: scale(1.3);opacity: 0}}.fadein{  animation: fadein 1s both;  -webkit-animation: fadein 1s both;}  @keyframes fadein{    0%{opacity: 0;}    100%{opacity: 1;}}@-webkit-keyframes fadein{  0%{opacity: 0;}  100%{opacity: 1;}}.fadeout{  animation: fadeout 1s both;  -webkit-animation: fadeout 1s both;}@keyframes fadeout{  0%{opacity: 1;}    100%{opacity: 0;}}@-webkit-keyframes fadeout{  0%{opacity: 1;}  100%{opacity: 0;}}.heartbeat{  animation: heartbeat 2s both infinite;  -webkit-animation: heartbeat 2s both infinite;}@keyframes heartbeat{  0%{transform: scale(1);}  8%{transform: scale(0.7); }  17%{transform: scale(1.5);}  25%{transform: scale(0.7);}  87.5%{transform: scale(0.7);}  100% {transform: scale(1);}}@-webkit-keyframes heartbeat{  0%{transform: scale(1);}  8%{transform: scale(0.7); }  17%{transform: scale(1.5);}  25%{transform: scale(0.7);}  87.5%{transform: scale(0.7);}  100% {transform: scale(1);}}    .egg-hide{        display: none !important;    }.s-ps-islite #doodle{    bottom:19px !important;}#doodle{    bottom:10px;';$("#egg-style").length<=0&&I.mountNode.append(e)}function c(){s(),I.mountNode.append(l()).append(u()),setTimeout(function(){z.mainContainer.show(),z.playerOuter.show()},50)}function l(){return z.mainContainer=$("<div></div>").css({width:I.width,height:I.height,position:"absolute",top:I.offsetTop+"px",left:0,right:0,margin:"auto","z-index":I.zIndex+1}),z.mainContainer.append(m()).append(f()).append(h()).append(p()).hide(),z.mainContainer}function p(){return z.video=$("<video></video>").attr("src",_.videoUrl).css({width:I.width+"px",height:I.height+"px",display:"none"}).hide(),z.video}function m(){return z.canvas=$('<canvas id="canvas"></canvas>').css({width:I.width,height:I.height,position:"absolute",left:0,right:0,margin:"auto","z-index":I.zIndex+3,"border-radius":"30px","-webkit-border-radius":"30px","-moz-border-radius":"30px"}),g(),z.canvas}function g(){var i=new Image;i.src=e(_.videoPoster),i.onload=function(){var e=document.getElementById("canvas");e.width=I.mediaW,e.height=I.mediaH;var t=e.getContext("2d");t.drawImage(i,0,0,I.mediaW,I.mediaH)}}function f(){return z.endingImage=$("<img>").attr("src",e(_.endingCover)).css({width:I.width+"px",height:I.height+"px",position:"absolute",left:0,right:0,margin:"auto",display:"none",zIndex:I.zIndex+1,borderRadius:"30px"}),z.endingImage}function h(){return z.qrCode=$('<img class="fadein">').attr("src",e(_.qrCode)).css({width:"81px",height:"auto",position:"absolute",right:"38px",top:"209px",zIndex:I.zIndex+2}),z.qrCode}function u(){return z.playerOuter=$("<div></div>").css({width:I.playerWidth+"px",height:I.playerHeight+"px",position:"absolute",top:I.offsetTop-6+"px",left:0,right:0,margin:"auto","z-index":I.zIndex+2,background:"url("+e(_.playerOuter)+") no-repeat top center","background-size":"100% 100%","border-radius":"30px"}),z.playerOuter.append(v()).append(b()).hide(),z.playerOuter}function v(){return z.btnControlBtn=$("<div></div>").css({position:"absolute",left:0,right:0,margin:"auto",width:I.width,"z-index":I.zIndex+4,"text-align":"right",padding:"12px 12px 0px 0px","box-sizing":"border-box","-moz-box-sizing":"border-box","-webkit-box-sizing":"border-box"}).append(),z.btnPlay=$('<img src="'+e(_.btnMusicOn)+'">').css({width:"40px",display:"inline-block","margin-right":"-6px",cursor:"pointer"}).click(function(){o({rsv_act:"music_btn"}),z.video[0].muted=!z.video[0].muted;var i=e(z.video[0].muted?_.btnMusicOff:_.btnMusicOn);$(this).attr("src",i)}),z.close=$('<img src="'+e(_.btnClose)+'">').css({width:"40px",display:"inline-block",cursor:"pointer"}).click(function(){o({rsv_act:"close_btn"}),a()}),z.vp1=$('<div id="videoP1" class="egg-hide" style="position: absolute;top: 6px;left: 192px;width: 100px;height: 100px;cursor:pointer;"><div class="circle-1"></div><div class="circle-2"></div><div class="circle-3"></div></div>'),z.vp2=$('<div id="videoP2" class="egg-hide" style="position: absolute;top: 51px;left: 97px;width: 100px;height: 100px;cursor:pointer;"><div class="circle-1"></div><div class="circle-2"></div><div class="circle-3"></div></div>'),z.vp1.click(function(){o({rsv_act:"videoP1_btn"}),$(this).addClass("egg-hide"),z.videoP=2,z.video[0].play()}),z.vp2.click(function(){o({rsv_act:"videoP2_btn"}),$(this).addClass("egg-hide"),z.videoP=3,z.video[0].play()}),z.btnControlBtn.append(z.btnPlay).append(z.close).append(z.vp1).append(z.vp2)}function b(){return z.shareBtn=$("<div></div>").css({height:"40px",position:"absolute",top:"175px",margin:"auto",left:"142px","z-index":I.zIndex+2,cursor:"pointer",display:"none","text-align":"center"}),z.btnEnter=$('<img class="fadein" src="'+e(_.btnEnter)+'">').css({width:"140px",height:"auto"}).on("click",function(){o({rsv_act:"redirect_btn"}),I.landPageUrl&&window.open(I.landPageUrl,"_target")}),z.btnReplay=$('<img class="fadein" src="'+e(_.btnReplay)+'">').css({width:"140px",height:"auto","margin-left":"10px"}).on("click",function(){o({rsv_act:"replay_btn"}),x(!0)}),z.shareBtn.append(z.btnShare).append(z.btnEnter).append(z.btnReplay),z.shareBtn}function x(e){var i=document.getElementById("canvas"),t=z.video;i.width=I.mediaW,i.height=I.mediaH;var o=i.getContext("2d");e&&(I.timer=null,I.videoTimer=null,z.videoP=1,t[0].currentTime=0,z.canvas.show(),z.shareBtn.hide()),t[0].play();var a=0;t[0].addEventListener("play",function(){null==I.timer&&(I.timer=window.setInterval(function(){a+=20,a>=0&&o.drawImage(t[0],0,0,I.mediaW,I.mediaH)},20)),null==I.videoTimer&&n()&&(I.timer=window.setInterval(function(){1==z.videoP&&t[0].currentTime>=18.5?($("#videoP1").removeClass("egg-hide"),t[0].pause()):2==z.videoP&&t[0].currentTime>=27+14/24&&($("#videoP2").removeClass("egg-hide"),t[0].pause())},10))},!1),t[0].addEventListener("ended",function(){clearInterval(I.timer),clearInterval(I.videoTimer),I.timer=null,I.videoTimer=null,1==z.videoP,z.shareBtn.show(),z.canvas.hide(),z.endingImage.show()},!1)}function y(){for(var t=[],n=0;n<P.length;n++)t.push(e(P[n]));t.push(e(i()?_.framesWhite:_.frames));for(var o=0,n=0;n<t.length;n++){var a=new Image;$(a).on("load error",function(){o++,o===t.length&&setTimeout(function(){d()},500)}),a.src=t[n]}}function w(){$(window).on("s-skinon",function(){z.mainContainer&&(a(!0),y())})}function k(){s(),o({fm:"pv"}),y(),w()}var I={baseUrl:"https://www.baidu.com/cache/yunying/worldearthday/",zIndex:5,timer:null,videoTimer:null,width:569,height:320,playerHeight:333,playerWidth:581,mediaW:1920,mediaH:1080,doodleW:270,doodleH:129,animationTimer:null,mountNode:$("body"),offsetTop:$("#form").offset().top+40+$("#form").height(),doodleHoverText:$("map[name=mp]").find("area").attr("title"),landPageUrl:"http://logo.baidu.com/earthday/index.html"},_={videoUrl:"  http://baidulogo.cdn.bcebos.com/earth_day%2Fvideo",videoPoster:"img/poster-pc.jpg",endingCover:"img/pc-end.jpg",btnMusicOn:"img/music-on.png",btnMusicOff:"img/music-off.png",btnClose:"img/close.png",playerOuter:"img/outer.png",btnEnter:"img/btn-enter.png",btnReplay:"img/btn-replay.png",qrCode:"img/qrcode.png",frames:"img/frame.gif",framesWhite:"img/frameWhite.gif",doodlePng:"img/doodle.png",doodlePngW:"img/doodle_w.png"},P=["img/poster-pc.jpg","img/pc-end.jpg","img/music-on.png","img/music-off.png","img/close.png","img/outer.png","img/btn-enter.png","img/btn-replay.png","img/qrcode.png","img/doodle.png","img/doodle_w.png"],z={};z.videoP=1,$(window).one("swap_begin",function(){a()}),$(window).on("index_off.indexLogoOff",function(){a()}),k()}();