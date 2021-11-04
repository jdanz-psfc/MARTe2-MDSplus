function plot_beam_geom(RABBIT_params)

%%
pp = RABBIT_params.Value;

clf;
if exist('TCV_view_TOR','file')
    TCV_view_TOR('tlsgc',0,0);
else
    plot3([0 0],[0 0],[-1,1],'k--');
end
hold on;

xvec = pp.xvec;
quiver3(pp.xstart(1,:),pp.xstart(2,:),pp.xstart(3,:),xvec(1,:),xvec(2,:),xvec(3,:));
axis equal;
%%

return