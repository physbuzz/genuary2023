(*colornames=Join@@(ColorData/@ColorData[]);*)
colornames=Join@@(ColorData/@ColorData[][[{1,4}]]);
daystrfunc[i_]:="Day "<>StringPadLeft[ToString[i],2,"0"]<>"\r(placeholder)";
fnamefunc[i_]:="day"<>StringPadLeft[ToString[i],2,"0"]<>"placeholder.png";
saveImage[ii_]:=Module[{img,blurred,imdata,imw,imh,pix,grad,convplot,daystr,fname,oimg},
daystr=daystrfunc[ii];
fname=fnamefunc[ii];
img=Rasterize[Graphics[{Text[Style[daystr,FontSize->50,Bold,Black],{0,0}]},PlotRange->{{-1,1},{-1,1}}]];
blurred=GaussianFilter[img,30];
imdata=ImageData[blurred];
{imw,imh,pix}=Dimensions[imdata];
Clear[grad];
grad[x_?NumberQ,y_?NumberQ]:=Module[{i,j,iprime,jprime},
i=Mod[Floor[(x+1)/2 imw],imw]+1;
j=Mod[Floor[(y+1)/2 imh],imh]+1;
iprime=Mod[i,imw]+1;
jprime=Mod[j,imh]+1;
{imdata[[iprime,j,1]]-imdata[[i,j,1]],imdata[[i,jprime,1]]-imdata[[i,j,1]]}
];
convplot=LineIntegralConvolutionPlot[100RotationMatrix[Pi/2+0.3].DiagonalMatrix[{-1,1}].grad[-x,y],{y,-1,1},{x,-1,1},Evaluated->False,Axes->False,Frame->False,PlotRangePadding->0,ImagePadding->0,Background->None,ColorFunction->ColorData[RandomChoice[colornames]]];
oimg=Rasterize[Show[convplot,Graphics[{Text[Style[daystr,FontSize->25,Bold,Black],{0,0}]},PlotRange->{{-1,1},{-1,1}}],PlotRange->{{-1,1},{-0.5,0.5}},AspectRatio->1/2],ImageSize->{200,100}];
Export[fname,oimg]
];

SetDirectory["~/genuary/placeholder/"];
ParallelTable[saveImage[k], {k, 1, 31}]


(* Simple code for generating the image embeds. *)
(*  
StringRiffle[ Riffle[StringSplit[Import["~/genuary/placeholder/tmp.txt"],"\n"],
Table["![placeholder "<>ToString[i]<>"](placeholder/"<>fnamefunc[i]<>")",{i,1,31}]],"\r"]
*)
