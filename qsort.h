#ifndef QSORT_H_
#define QSORT_H_

const double LOG2=0.69314718;

#include<math.h>

struct QSortStack
{
	size_t left,right;
};

template<class TYPE>
inline void swap(TYPE &a,TYPE &b,TYPE &c)
{
	c=a;a=b;b=c;
}
template<class TYPE>
void quick_sort(TYPE a[],size_t n)
{
	if (n == 0) return;
	size_t s,t,i,j,k;
	QSortStack *top,*stack;
	stack=(QSortStack*)malloc(sizeof(QSortStack)
			*(size_t)((sizeof(size_t)*log(n)/LOG2)+2));
	TYPE rp,tmp;

	top=stack;s=0;t=n-1;
	while(1) {
		if(s<t) {
			i=s;j=t;k=(i+j)>>1;rp=a[k];
			swap(a[k],a[t],tmp);
			do {
				while(a[++i]<rp);
				while (j && rp < a[--j]);
				swap(a[i],a[j],tmp);
			} while(i<j);
			swap(a[i],a[j],tmp);
			swap(a[i],a[t],tmp);
			if(i-s>t-i) {
				if(i-s>9) { top->left=s;top->right=i-1;top++; }
				if(t-i>9) s=i+1;
					else s=t;
			} else {
				if(t-i>9) { top->left=i+1;top->right=t;top++; }
				if(i-s>9) t=i-1;
					else t=s;
			}
		} else {
			if(top==stack) {
				free(stack);
				for(i=1;i<n;i++)
					for(j=i;j>0 && a[j]<a[j-1];j--)
						swap(a[j],a[j-1],tmp);
				return;
			} else { top--;s=top->left;t=top->right; }
		}
	}
}

template<class TYPE, class CMP_CLASS>
void quick_sort(TYPE a[], size_t n, const CMP_CLASS &cc) // cc is a pseudo function for comparison
{
	if (n == 0) return;
	size_t s,t,i,j,k;
	QSortStack *top,*stack;
	stack=(QSortStack*)malloc(sizeof(QSortStack)
			*(size_t)((sizeof(size_t)*log(n)/LOG2)+2));
	TYPE rp,tmp;

	top=stack;s=0;t=n-1;
	while(1) {
		if(s<t) {
			i=s;j=t;k=(i+j)>>1;rp=a[k];
			swap(a[k],a[t],tmp);
			do {
				while(cc(a[++i], rp));
				while (j && cc(rp, a[--j]));
				swap(a[i],a[j],tmp);
			} while(i<j);
			swap(a[i],a[j],tmp);
			swap(a[i],a[t],tmp);
			if(i-s>t-i) {
				if(i-s>9) { top->left=s;top->right=i-1;top++; }
				if(t-i>9) s=i+1;
					else s=t;
			} else {
				if(t-i>9) { top->left=i+1;top->right=t;top++; }
				if(i-s>9) t=i-1;
					else t=s;
			}
		} else {
			if(top==stack) {
				free(stack);
				for(i=1;i<n;i++)
					for(j=i;j>0 && cc(a[j],a[j-1]);j--)
						swap(a[j],a[j-1],tmp);
				return;
			} else { top--;s=top->left;t=top->right; }
		}
	}
}

#endif // QUICK_SORT_H_
