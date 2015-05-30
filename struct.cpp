#include<cstdio>
#include<cstdlib>
#include<cstring>

typedef struct {
	unsigned char r,g,b;
} RGB;
typedef enum { NO_NODE, LR_NODE, UD_NODE, ALL_NODE } node_status;
typedef struct _node {
	node_status stat;
	int r,c;
	int h,w;
	union _u {
		struct _node* child;
		RGB color;
	} u;
} node;

void free_tree(node tree)
{
	switch(tree.stat)
	{
		case ALL_NODE:
			for(int i=0;i<4;++i)
				free_tree(tree.u.child[i]);
			free(tree.u.child);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				free_tree(tree.u.child[i]);
			free(tree.u.child);
			break;
		case NO_NODE:
			break;
		default:
			puts("Error caught in free_tree.");
	}
}

void tree_to_array(RGB** img, node tree)
{
	switch(tree.stat)
	{
		case ALL_NODE:
			for(int i=0;i<4;++i)
				tree_to_array(img,tree.u.child[i]);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				tree_to_array(img,tree.u.child[i]);
			break;
		case NO_NODE:
			for(int i=tree.r;i<tree.r+tree.h;++i)
				for(int j=tree.c;j<tree.c+tree.w;++j)
					img[i][j] = tree.u.color;
			break;
		default:
			puts("Error caught in tree_to_array.");
	}
}

node make_tree(RGB** img, int r, int c, int h, int w)
{
	node base;
	int i,j;

	for(i=r;i<r+h;++i)
	{
		for(j=c;j<c+w;++j)
			if(img[i][j].r!=img[r][c].r || img[i][j].g!=img[r][c].g || img[i][j].b!=img[r][c].b)
				break;
		if(j!=c+w)
			break;
	}
	base.r = r;
	base.c = c;
	base.h = h;
	base.w = w;
	if(i==r+h&&j==c+w)
	{
		base.stat = NO_NODE;
		base.u.color = img[r][c];
	}
	else
	{
		if(w==1)
		{
			base.stat = UD_NODE;
			base.u.child = (node*) malloc(sizeof(node)*2);
			base.u.child[0] = make_tree(img,r,c,h/2,w);
			base.u.child[1] = make_tree(img,r+h/2,c,h-h/2,w);
		}
		else if(h==1)
		{
			base.stat = LR_NODE;
			base.u.child = (node*) malloc(sizeof(node)*2);
			base.u.child[0] = make_tree(img,r,c,h,w/2);
			base.u.child[1] = make_tree(img,r,c+w/2,h,w-w/2);
		}
		else
		{
			base.stat = ALL_NODE;
			base.u.child = (node*) malloc(sizeof(node)*4);
			base.u.child[0] = make_tree(img,r,c,h/2,w/2);
			base.u.child[1] = make_tree(img,r+h/2,c,h-h/2,w/2);
			base.u.child[2] = make_tree(img,r,c+w/2,h/2,w-w/2);
			base.u.child[3] = make_tree(img,r+h/2,c+w/2,h-h/2,w-w/2);
		}
	}
	return base;
}

node rgb_ratio(node tree,float r_weight,float g_weight,float b_weight)
{
	switch(tree.stat)
	{
		case ALL_NODE:
			for(int i=0;i<4;++i)
				tree.u.child[i] = rgb_ratio(tree.u.child[i],r_weight,g_weight,b_weight);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				tree.u.child[i] = rgb_ratio(tree.u.child[i],r_weight,g_weight,b_weight);
			break;
		case NO_NODE:
			float r,g,b;
			r = ((float) tree.u.color.r) * r_weight;
			g = ((float) tree.u.color.g) * g_weight;
			b = ((float) tree.u.color.b) * b_weight;
			if(r<0) r = 0;
			if(g<0) g = 0;
			if(b<0) b = 0;
			if(r>255) r = 255;
			if(g>255) g = 255;
			if(b>255) b = 255;
			tree.u.color.r = (unsigned char) r;
			tree.u.color.g = (unsigned char) g;
			tree.u.color.b = (unsigned char) b;
			break;
		default:
			puts("Error caught in rgb_ratio.");
	}
	return tree;
}

node rgb_relation(node tree,int r_relation[256],int g_relation[256],int b_relation[256])
{
	int r,g,b;
	switch(tree.stat)
	{
		case ALL_NODE:
			for(int i=0;i<4;++i)
				tree.u.child[i] = rgb_relation(tree.u.child[i],r_relation,g_relation,b_relation);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				tree.u.child[i] = rgb_relation(tree.u.child[i],r_relation,g_relation,b_relation);
			break;
		case NO_NODE:
			r = r_relation[tree.u.color.r];
			g = g_relation[tree.u.color.g];
			b = b_relation[tree.u.color.b];
			if(r<0) r = 0;
			if(g<0) g = 0;
			if(b<0) b = 0;
			if(r>255) r = 255;
			if(g>255) g = 255;
			if(b>255) b = 255;
			tree.u.color.r = r;
			tree.u.color.g = g;
			tree.u.color.b = b;
			break;
		default:
			puts("Error caught in rgb_relation.");
	}
	return tree;
}

void make_hist(node tree, int hr[256], int hb[256], int hg[256])
{
	switch(tree.stat)
	{
		case NO_NODE:
			hr[tree.u.color.r] += tree.w*tree.h;
			hg[tree.u.color.g] += tree.w*tree.h;
			hb[tree.u.color.b] += tree.w*tree.h;
			break;
		case ALL_NODE:
			for(int i=0;i<4;++i)
				make_hist(tree.u.child[i],hr,hb,hg);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				make_hist(tree.u.child[i],hr,hb,hg);
			break;
		default:
			puts("Error caught in make_hist.");
	}
}

node hist_eq(node tree)
{
	int hr[256] = {0,};
	int hb[256] = {0,};
	int hg[256] = {0,};
	int cr[256] = {0,};
	int cb[256] = {0,};
	int cg[256] = {0,};
	int i,j;

	make_hist(tree,hr,hb,hg);

	// smooth histogram (optional)
	{
		hr[0] = (hr[0]+hr[1])/2;
		hr[255] = (hr[254]+hr[255])/2;
		for(i=1;i<255;++i) hr[i] = (hr[i-1]+hr[i]+hr[i+1])/3;
		hg[0] = (hg[0]+hg[1])/2;
		hg[255] = (hg[254]+hg[255])/2;
		for(i=1;i<255;++i) hg[i] = (hg[i-1]+hg[i]+hg[i+1])/3;
		hb[0] = (hb[0]+hb[1])/2;
		hb[255] = (hb[254]+hb[255])/2;
		for(i=1;i<255;++i) hb[i] = (hb[i-1]+hb[i]+hb[i+1])/3;
	}

	cr[0] = hr[0];
	cb[0] = hb[0];
	cg[0] = hg[0];
	for(i=1;i<256;++i)
	{
		cr[i] += cr[i-1]+hr[i];
		cg[i] += cg[i-1]+hg[i];
		cb[i] += cb[i-1]+hb[i];
	}
	for(i=0;i<256;++i)
	{
		cr[i] *= 256;
		cr[i] /= tree.w*tree.h;
		if(cr[i]<0) cr[i] = 0;
		if(cr[i]>255) cr[i] = 255;
		cg[i] *= 256;
		cg[i] /= tree.w*tree.h;
		if(cg[i]<0) cg[i] = 0;
		if(cg[i]>255) cg[i] = 255;
		cb[i] *= 256;
		cb[i] /= tree.w*tree.h;
		if(cb[i]<0) cb[i] = 0;
		if(cb[i]>255) cb[i] = 255;
	}

	tree = rgb_relation(tree,cr,cb,cg);

	return tree;
}

int main(void)
{
	unsigned char hdr[54];
	unsigned char *dat;
	int width, height, padding;
	int i,j;
	FILE *out, *fp = fopen("marbles.bmp","rb");
	RGB **img;

	fread(hdr,sizeof(unsigned char),54,fp);

	width = *(int*)&hdr[18];
	height = *(int*)&hdr[22];
	for(padding=0;(width*3+padding)%4;++padding);
	dat = (unsigned char*)malloc(sizeof(unsigned char)*((width*3)+padding));
	img = (RGB**)malloc(sizeof(RGB*)*height);
	for(i=0;i<width;++i)
		img[i] = (RGB*)malloc(sizeof(RGB)*width);
	/*====== 초기화 ======*/

	for(i=0;i<height;++i)
	{
		fread(dat,sizeof(unsigned char),width*3+padding,fp); /* 이미지는 bgr 순서로 저장됩니다. */
		for(j=0;j<width*3;j+=3)
		{
			img[height-i-1][j/3].b = dat[j];
			img[height-i-1][j/3].g = dat[j+1];
			img[height-i-1][j/3].r = dat[j+2];
		}
	}
	fclose(fp);

	node n = make_tree(img,0,0,height,width);
//	n = rgb_ratio(n,0,1,0);
	n = hist_eq(n);
	tree_to_array(img,n);
	free_tree(n);
	
	fp = fopen("marbles.bmp","rb");
	out = fopen("change.bmp","wb");
	fread(hdr,sizeof(unsigned char),54,fp);
	fwrite(hdr,sizeof(unsigned char),54,out);
	for(i=0;i<height;++i)
	{
		memset(dat,0,((width*3)+padding)*sizeof(unsigned char));
		for(j=0;j<width*3;j+=3)
		{
			dat[j] = img[height-i-1][j/3].b;
			dat[j+1] = img[height-i-1][j/3].g;
			dat[j+2] = img[height-i-1][j/3].r;
		}
		fwrite(dat,sizeof(unsigned char),width*3+padding,out);
	}
	fclose(fp);
	return 0;
}

