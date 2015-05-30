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
	int w,h;
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
		case NO_NODE:
			for(int i=0;i<tree.h;++i)
				for(int j=0;j<tree.w;++j)
					img[i+tree.r][j+tree.c] = tree.u.color;
			break;
		case ALL_NODE:
			for(int i=0;i<4;++i)
				tree_to_array(img,tree.u.child[i]);
			break;
		case LR_NODE:
		case UD_NODE:
			for(int i=0;i<2;++i)
				tree_to_array(img,tree.u.child[i]);
			break;
		default:
			puts("Error caught in tree_to_array.");
	}
}

node make_tree(RGB** img, int r, int c, int w, int h){
	node base;
	int i,j;

	for(i=r;i<r+h;++i)
		for(j=c;j<c+w;++j)
			if(img[i][j].r!=img[r][c].r || img[i][j].g!=img[r][c].g || img[i][j].b!=img[r][c].b)
				break;
	base.r = r;
	base.c = c;
	base.w = w;
	base.h = h;
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
			base.u.child[0] = make_tree(img,r,c,w,h/2);
			base.u.child[1] = make_tree(img,r,c+h/2,w,h-h/2);
		}
		else if(h==1)
		{
			base.stat = LR_NODE;
			base.u.child = (node*) malloc(sizeof(node)*2);
			base.u.child[0] = make_tree(img,r,c,w/2,h);
			base.u.child[1] = make_tree(img,r+w/2,c,w-w/2,h);
		}
		else
		{
			base.stat = ALL_NODE;
			base.u.child = (node*) malloc(sizeof(node)*4);
			base.u.child[0] = make_tree(img,r,c,w/2,h/2);
			base.u.child[1] = make_tree(img,r+w/2,c,w-w/2,h/2);
			base.u.child[2] = make_tree(img,r,c+h/2,w/2,h-h/2);
			base.u.child[3] = make_tree(img,r+w/2,c+h/2,w-w/2,h-h/2);
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
	img = (RGB**)malloc(sizeof(RGB*)*width);
	for(i=0;i<width;++i)
		img[i] = (RGB*)malloc(sizeof(RGB)*height);
	/*====== 초기화 ======*/

	for(i=0;i<height;++i)
	{
		fread(dat,sizeof(unsigned char),width*3+padding,fp); /* 이미지는 bgr 순서로 저장됩니다. */
		for(j=0;j<width*3;j+=3)
		{
			img[j/3][height-i-1].b = dat[j];
			img[j/3][height-i-1].g = dat[j+1];
			img[j/3][height-i-1].r = dat[j+2];
		}
	}
	fclose(fp);
	
	node n = make_tree(img,0,0,width,height);
	n = rgb_ratio(n,0.5,0.5,1);
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
			dat[j] = img[j/3][height-i-1].b;
			dat[j+1] = img[j/3][height-i-1].g;
			dat[j+2] = img[j/3][height-i-1].r;
		}
		fwrite(dat,sizeof(unsigned char),width*3+padding,out);
	}
	fclose(fp);
	return 0;
}

