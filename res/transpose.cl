//This kernel will transpose matrices that are size 4nx4n
//g_mat is the global matrix, l_mat is the local matrix that we use to store a copy of the 
//non-transposed matrix to then write the transposed values back to global
//Should I make size, block size and block per row __constants?
__kernel void transpose(__global float4 *g_mat, __local float4 *l_mat, uint size){
   //Determine row and column location
   int col = get_global_id(0);
   int row = 0;
   while(col >= size) {
      col -= size--;
      row++;
   }
   col += row;
   size += row;

   __global float4 *src, *dst;
   //Read source block into local memory, note that we multiply by 4
   //to get the matrix dimension b/c size is the size of the matrix in 4x4 blocks ie.
   //size = matDim / 4
   src = g_mat + row * size * 4 + col;
   l_mat += get_local_id(0)*8;
   l_mat[0] = src[0];
   l_mat[1] = src[size];
   l_mat[2] = src[2*size];
   l_mat[3] = src[3*size];

   //Tranpose block on diagonal
   if(row == col) {
      src[0] = 
         (float4)(l_mat[0].x, l_mat[1].x, l_mat[2].x, l_mat[3].x);
      src[size] = 
         (float4)(l_mat[0].y, l_mat[1].y, l_mat[2].y, l_mat[3].y);
      src[2*size] = 
         (float4)(l_mat[0].z, l_mat[1].z, l_mat[2].z, l_mat[3].z);
      src[3*size] = 
         (float4)(l_mat[0].w, l_mat[1].w, l_mat[2].w, l_mat[3].w);
   }
   //Transpose off diagonal blocks, we find the corresponding block on the opposite
   //side of the matrix and swap with it
   else {
      //Read destination block into local memory
      dst = g_mat + col * size * 4 + row;
      l_mat[4] = dst[0];
      l_mat[5] = dst[size];
      l_mat[6] = dst[2*size];
      l_mat[7] = dst[3*size];

      //Set elements of destination block
      dst[0] = 
         (float4)(l_mat[0].x, l_mat[1].x, l_mat[2].x, l_mat[3].x);
      dst[size] = 
         (float4)(l_mat[0].y, l_mat[1].y, l_mat[2].y, l_mat[3].y);
      dst[2*size] = 
         (float4)(l_mat[0].z, l_mat[1].z, l_mat[2].z, l_mat[3].z);
      dst[3*size] = 
         (float4)(l_mat[0].w, l_mat[1].w, l_mat[2].w, l_mat[3].w);

      //Set elements of source block
      src[0] = 
         (float4)(l_mat[4].x, l_mat[5].x, l_mat[6].x, l_mat[7].x);
      src[size] = 
         (float4)(l_mat[4].y, l_mat[5].y, l_mat[6].y, l_mat[7].y);
      src[2*size] = 
         (float4)(l_mat[4].z, l_mat[5].z, l_mat[6].z, l_mat[7].z);
      src[3*size] = 
         (float4)(l_mat[4].w, l_mat[5].w, l_mat[6].w, l_mat[7].w);
   }
}

